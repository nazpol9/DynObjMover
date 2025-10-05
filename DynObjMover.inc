/************************************************************
 * DynObjMover.inc
 * * Dynamic Object Mover for SA-MP
 
 * Author: Brian Randall

 * Features:
 * - Move objects with position + rotation
 * - Queue system for flood-safe movement
 * - Auto-close objects after timer
 * - Optional console logging

 ************************************************************/

#define MAX_DYN_OBJECTS 1000
#define QUEUE_MAX 1000
#define QUEUE_INTERVAL 50
#define FLOOD_COOLDOWN 1000

new bool:LOG_DYN = true;

// ------------------- DYNAMIC OBJECT DATA -------------------
enum E_DynMoveData
{
    isOpen,
    isInit,
    Float:startX,
    Float:startY,
    Float:startZ,
    Float:startRX,
    Float:startRY,
    Float:startRZ,
    Float:endX,
    Float:endY,
    Float:endZ,
    Float:endRX,
    Float:endRY,
    Float:endRZ,
    Float:lastTargetX,
    Float:lastTargetY,
    Float:lastTargetZ,
    Float:lastTargetRX,
    Float:lastTargetRY,
    Float:lastTargetRZ,
    closeTimer, // timer:
    lastMoveTick // int
}

// ------------------- ARRAYS -------------------
new MoverDyn_DynObjectID[MAX_DYN_OBJECTS];

new Float:MoverDyn_Data[MAX_DYN_OBJECTS][E_DynMoveData];

// ------------------- MOVE QUEUE -------------------
new queueStart = 0;
new queueEnd = 0;
new queueTimer = 0;

new MoverDyn_Queue_ID[QUEUE_MAX];
new Float:MoverDyn_Queue_X[QUEUE_MAX];
new Float:MoverDyn_Queue_Y[QUEUE_MAX];
new Float:MoverDyn_Queue_Z[QUEUE_MAX];
new Float:MoverDyn_Queue_RX[QUEUE_MAX];
new Float:MoverDyn_Queue_RY[QUEUE_MAX];
new Float:MoverDyn_Queue_RZ[QUEUE_MAX];
new Float:MoverDyn_Queue_MoveSpeed[QUEUE_MAX];

// ------------------- MAX ACTIVE DYNAMIC OBJECTS -------------------
new MAX_ACTIVE_DYN = MAX_DYN_OBJECTS;

// ------------------- FORWARD -------------------
forward ProcessMoveQueue();
forward DynAutoClose(dynobjectid);

// ------------------- INITIALIZATION -------------------
stock DynObjMover_Init()
{
    MAX_ACTIVE_DYN = MAX_DYN_OBJECTS;

    if(LOG_DYN) printf("[DynObjMover] MAX_ACTIVE_DYN set to %d", MAX_ACTIVE_DYN);
}

// ------------------- CORE -------------------
stock GetDynIndex(dynobjectid)
{
    for(new i = 0; i < MAX_ACTIVE_DYN; i++)
    {
        if(MoverDyn_DynObjectID[i] == dynobjectid) return i;
        else if(MoverDyn_DynObjectID[i] == 0)
        {
            MoverDyn_DynObjectID[i] = dynobjectid;
            return i;
        }
    }
    return -1;
}

// ------------------- ENQUEUE -------------------
stock EnqueueMove(dynobjectid, Float:X, Float:Y, Float:Z, Float:RX, Float:RY, Float:RZ, Float:moveSpeed)
{
    new next = (queueEnd + 1) % QUEUE_MAX;
    if(next == queueStart)
    {
        if(LOG_DYN) printf("[DynObjMover] Queue is full! Object %d move cancelled.", dynobjectid);
        return 0;
    }

    MoverDyn_Queue_ID[queueEnd] = dynobjectid;
    MoverDyn_Queue_X[queueEnd] = X;
    MoverDyn_Queue_Y[queueEnd] = Y;
    MoverDyn_Queue_Z[queueEnd] = Z;
    MoverDyn_Queue_RX[queueEnd] = RX;
    MoverDyn_Queue_RY[queueEnd] = RY;
    MoverDyn_Queue_RZ[queueEnd] = RZ;
    MoverDyn_Queue_MoveSpeed[queueEnd] = moveSpeed;

    queueEnd = next;

    if(queueTimer == 0)
        queueTimer = SetTimer("ProcessMoveQueue", QUEUE_INTERVAL, true);

    return 1;
}

// ------------------- PROCESS QUEUE -------------------
public ProcessMoveQueue()
{
    if(queueStart == queueEnd)
    {
        KillTimer(queueTimer);
        queueTimer = 0;
        return 1;
    }

    new intObj = MoverDyn_Queue_ID[queueStart];

    if(IsValidDynamicObject(intObj))
    {
        MoveDynamicObject(intObj,
            MoverDyn_Queue_X[queueStart],
            MoverDyn_Queue_Y[queueStart],
            MoverDyn_Queue_Z[queueStart],
            MoverDyn_Queue_MoveSpeed[queueStart],
            MoverDyn_Queue_RX[queueStart],
            MoverDyn_Queue_RY[queueStart],
            MoverDyn_Queue_RZ[queueStart]
        );

        if(LOG_DYN)
            printf("[DynObjMover] Moved object %d to (%.2f, %.2f, %.2f)", intObj, MoverDyn_Queue_X[queueStart], MoverDyn_Queue_Y[queueStart], MoverDyn_Queue_Z[queueStart]);
    }
    else
    {
        if(LOG_DYN) printf("[DynObjMover] Failed to move object %d (invalid ID)", intObj);
    }

    queueStart = (queueStart + 1) % QUEUE_MAX;
    return 1;
}

// ------------------- AUTO CLOSE -------------------
public DynAutoClose(dynobjectid)
{
    if(!IsValidDynamicObject(dynobjectid)) return 0;

    new index = GetDynIndex(dynobjectid);
    if(index == -1) return 0;
    
    if(floatround(MoverDyn_Data[index][isOpen]))
    {
        EnqueueMove(dynobjectid,
            MoverDyn_Data[index][startX],
            MoverDyn_Data[index][startY],
            MoverDyn_Data[index][startZ],
            MoverDyn_Data[index][startRX],
            MoverDyn_Data[index][startRY],
            MoverDyn_Data[index][startRZ],
            1.0
        );

        MoverDyn_Data[index][isOpen] = 0;
        MoverDyn_Data[index][closeTimer] = 0;

        if(LOG_DYN) printf("[DynObjMover] Auto-closed object %d", dynobjectid);
    }
    return 1;
}

// ------------------- TOGGLE SINGLE -------------------
stock ToggleDynamicObject(dynobjectid, Float:offsetX, Float:offsetY, Float:offsetZ, Float:moveSpeed, Float:offsetRX, Float:offsetRY, Float:offsetRZ, autoCloseTime)
{
    new dynarray[1];
    dynarray[0] = dynobjectid;
    return ToggleDynamicObjects(dynarray, 1, offsetX, offsetY, offsetZ, moveSpeed, offsetRX, offsetRY, offsetRZ, autoCloseTime);
}

// ------------------- TOGGLE MULTI -------------------
stock ToggleDynamicObjects(dynobjectid[], count, Float:X_END, Float:Y_END, Float:Z_END, Float:moveSpeed, Float:RX_END, Float:RY_END, Float:RZ_END, autoCloseTime)
{
    for(new i = 0; i < count; i++)
    {
        new obj = dynobjectid[i];
        if(!IsValidDynamicObject(obj)) continue;

        new index = GetDynIndex(obj);
        if(index == -1) continue;

        new isInit_val = MoverDyn_Data[index][isInit];
        new isOpen_val = MoverDyn_Data[index][isOpen];

        if(!isInit_val)
        {
            GetDynamicObjectPos(obj, MoverDyn_Data[index][startX], MoverDyn_Data[index][startY], MoverDyn_Data[index][startZ]);
            GetDynamicObjectRot(obj, MoverDyn_Data[index][startRX], MoverDyn_Data[index][startRY], MoverDyn_Data[index][startRZ]);

            MoverDyn_Data[index][endX] = X_END; 
            MoverDyn_Data[index][endY] = Y_END; 
            MoverDyn_Data[index][endZ] = Z_END; 

            MoverDyn_Data[index][endRX] = RX_END; 
            MoverDyn_Data[index][endRY] = RY_END; 
            MoverDyn_Data[index][endRZ] = RZ_END; 

            MoverDyn_Data[index][isInit] = 1;
        }

        new Float:targetX, Float:targetY, Float:targetZ;
        new Float:targetRX, Float:targetRY, Float:targetRZ;

        if(!isOpen_val)
        {
            targetX = MoverDyn_Data[index][endX];
            targetY = MoverDyn_Data[index][endY];
            targetZ = MoverDyn_Data[index][endZ];

            targetRX = MoverDyn_Data[index][endRX];
            targetRY = MoverDyn_Data[index][endRY];
            targetRZ = MoverDyn_Data[index][endRZ];

            MoverDyn_Data[index][isOpen] = 1;

            new timerID = floatround(MoverDyn_Data[index][closeTimer]);
            if(timerID != 0)
            {
                KillTimer(timerID);
                MoverDyn_Data[index][closeTimer] = 0;
            }

            if(autoCloseTime > 0)
            {
                new newTimerID = SetTimerEx("DynAutoClose", autoCloseTime * 1000, false, "i", obj);
                MoverDyn_Data[index][closeTimer] = newTimerID;
            }

            if(LOG_DYN) printf("[DynObjMover] Opened object %d", obj);
        }
        else
        {
            targetX = MoverDyn_Data[index][startX];
            targetY = MoverDyn_Data[index][startY];
            targetZ = MoverDyn_Data[index][startZ];

            targetRX = MoverDyn_Data[index][startRX];
            targetRY = MoverDyn_Data[index][startRY];
            targetRZ = MoverDyn_Data[index][startRZ];

            MoverDyn_Data[index][isOpen] = 0;

            new timerID = floatround(MoverDyn_Data[index][closeTimer]);
            if(timerID != 0)
            {
                KillTimer(timerID);
                MoverDyn_Data[index][closeTimer] = 0;
            }

            if(LOG_DYN) printf("[DynObjMover] Closed object %d", obj);
        }

        // Check if move is same as last
        if(targetX == MoverDyn_Data[index][lastTargetX] &&
           targetY == MoverDyn_Data[index][lastTargetY] &&
           targetZ == MoverDyn_Data[index][lastTargetZ] &&
           targetRX == MoverDyn_Data[index][lastTargetRX] &&
           targetRY == MoverDyn_Data[index][lastTargetRY] &&
           targetRZ == MoverDyn_Data[index][lastTargetRZ])
        {
            continue;
        }

        MoverDyn_Data[index][lastTargetX] = targetX;
        MoverDyn_Data[index][lastTargetY] = targetY;
        MoverDyn_Data[index][lastTargetZ] = targetZ;
        MoverDyn_Data[index][lastTargetRX] = targetRX;
        MoverDyn_Data[index][lastTargetRY] = targetRY;
        MoverDyn_Data[index][lastTargetRZ] = targetRZ;

        EnqueueMove(obj, targetX, targetY, targetZ, targetRX, targetRY, targetRZ, moveSpeed);
    }

    return 1;
}