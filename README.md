# DynObjMover: Dynamic Object Mover for SA-MP (Pawn)

[![SA-MP](https://img.shields.io/badge/SA--MP-0.3.7-blue.svg)](https://www.sa-mp.com/)

**DynObjMover.inc** is a robust, optimized, and flood-safe include for smoothly **moving and rotating dynamic objects** in SA-MP (using the Streamer plugin).

It's perfect for animating doors, gates, bridges, and any other movable world elements with precision and ease.

---

## 🌟 Key Features

* **Absolute Positioning:** Object movement is defined using **absolute final coordinates** (X, Y, Z, RX, RY, RZ), simplifying animation setup.
* **Toggle System:** Supports a reliable `toggle` function (open/close) with automatic return to the original starting position.
* **Flood Protection (Queue System):** All movement requests are queued and processed at a set interval (default 50ms), preventing server lag.
* **Reliable Auto-Close Timer:** Optional, stable timer to automatically return the object to its starting position after a specified duration.

---

## 🛠️ Requirements

1.  **SA-MP Server (0.3.7 or newer)**
2.  **Streamer Plugin (latest version)** by **Incognito**.

---

## 🚀 Installation and Usage

### Step 1: Initialization

Include the file and call the initialization function within `OnGameModeInit`.

```pawn
#include <a_samp>
#include <streamer>
#include <DynObjMover>

public OnGameModeInit()
{
    // ... your object creation code ...
    
    // Crucial initialization of the mover system!
    DynObjMover_Init();
    
    return 1;
}
```

### Step 2: Calling the Movement Function
Use the **ToggleDynamicObject** function.

**Important:** The function expects **absolute final coordinates** (where the object should end up). The object's starting coordinates are fixed automatically by the system.

```pawn
stock ToggleDynamicObject(
    dynobjectid, 
    Float:X_END, 
    Float:Y_END, 
    Float:Z_END, 
    Float:moveSpeed, 
    Float:RX_END, 
    Float:RY_END, 
    Float:RZ_END, 
    autoCloseTime // int: time in seconds (0 = disable auto-close)
)
```

### Usage Example (Moving an Object and Auto-Closing):

Suppose **PoliceGate** is created at **(1401.66, -831.14, 1104.41)** with a rotation **RZ: 90.0**.

```pawn
// First call: Move to X:1401.70, Y:-831.15 and set RZ to -13.4.
ToggleDynamicObject(
    PoliceGate, 
    1401.704711, // X_END (New final position X)
    -831.155273, // Y_END
    1104.419189, // Z_END
    0.3,         // moveSpeed
    0.0,         // RX_END
    0.0,         // RY_END
    -13.400012,  // RZ_END
    5            // autoCloseTime (Automatically closes after 5 seconds)
);
```

**First call:** The object moves to the specified X_END/Y_END/Z_END.
**After 5 seconds (or on the next toggle):** The object returns to its original starting position.

### ⚙️ Configuration
You can adjust these settings at the top of the include file:

| Variable | Default value | Description |
| :--- | :--- | :--- |
| `#define MAX_DYN_OBJECTS` | 1000 | Maximum number of dynamic objects the mover can track. |
| `#define QUEUE_INTERVAL` | 50 | The delay (in ms) between processing movement requests. |
| `new bool:LOG_DYN = true;` | `true` | Set to false to disable all movement logging in the server console. |

## 👨‍💻 Author
**Brian Randall** (Original concept and final stabilization)

All suggestions, bug reports, and Pull Requests are welcome!
