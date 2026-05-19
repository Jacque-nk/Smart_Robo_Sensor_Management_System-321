# Smart_Robo_Sensor_Management_System-321
Sensor Management System
# Smart Sensor Lifecycle Monitoring System
### A C++ Object-Oriented Programming Project 

---

## What This Project Does

This is a terminal-based sensor management system written in C++. It lets a logged-in user add, view, search, update, and delete sensors — with automatic alerts when a sensor reading goes outside its safe range, a full undo system, and a history of all sensors added.

It was built to model how a real engineering environment tracks sensors in robotic or automated systems.

---

## How to Run

### Requirements
- g++ compiler (MinGW on Windows, GCC on Linux/Mac)
- VS Code (optional, for F5 debugging)

### Step 1 — Create the credentials file
Before running for the first time, create an empty file called `users.txt` in the same folder as the `.cpp` file.

### Step 2 — Compile
```bash
g++ -g sensor_system_v2.cpp -o sensor_system_v2
```

### Step 3 — Run
```bash
# Windows
sensor_system_v2.exe

# Linux / Mac
./sensor_system_v2
```

> **Tip:** Run from a real terminal (not VS Code's built-in one) for the password asterisk masking to work correctly.

### Using VS Code
- `Ctrl+Shift+B` — builds the project (uses `tasks.json`)
- `F5` — runs with debugger (uses `launch.json`)
- Make sure `miDebuggerPath` in `launch.json` points to your `gdb.exe`

---

## Folder Structure

```
your_project/
├── .vscode/
│   ├── tasks.json            ← compiles with g++
│   └── launch.json           ← runs in external console
├── sensor_system_v2.cpp      ← the full program
├── users.txt                 ← credentials (create empty before first run)
└── README.md                 ← this file
```

---

## Menu Options

| Option | Name | What It Does |
|--------|------|--------------|
| 1 | Add Sensors | Enter how many sensors, then fill fields one by one (type `b` to go back, `q` to cancel) |
| 2 | Display All Sensors | Shows all sensors with name, ID, value, range, and alert status |
| 3 | Search Sensor | Finds a sensor by ID using binary search |
| 4 | Delete Sensor | Removes a sensor by ID (can be undone) |
| 5 | Update Sensor Value | Changes a sensor's reading (old value saved for undo) |
| 6 | History | Lists sensors in the order they were added |
| 7 | Undo Last Action | Reverses the last add, update, or delete exactly |
| 8 | Exit | Frees all memory and exits |

---

## How the Undo System Works

Every action saves a **snapshot** of the sensor before anything changes:

- **Undo an ADD** → removes the sensor that was just added, frees its memory
- **Undo an UPDATE** → restores the old value from the saved snapshot
- **Undo a DELETE** → puts the sensor back into the system (it was never freed)

This is real Ctrl+Z behaviour — not just deleting the last thing added.

---

## How the Step-Back Input Works

When adding a sensor, fields are collected one at a time:

```
Name          : TempSensor
ID            : 101
Value         : (type 'b' to go back)
→ goes back to ID field
ID            : 102
Value         : 36.5
Min threshold : 20
Max threshold : 80
```

Type `b` at any point to go back one field. Type `q` to cancel that sensor entirely.

---

## How Alerts Work

Every time a sensor is displayed, the program automatically checks:

```
if (value < minT || value > maxT) → *** ALERT: Value out of threshold! ***
```

No separate alert function is needed — it fires every single time a sensor is shown.

---

## Data Structures Used

| Structure | Type | Purpose |
|-----------|------|---------|
| `vector<Sensor*>` | Dynamic array of pointers | Stores all sensors |
| `stack<Action>` | LIFO stack | Undo history — stores snapshots |
| `queue<Sensor*>` | FIFO queue | Tracks sensors in add order for history |

---

## OOP Concepts Used

| Concept | Where |
|---------|-------|
| **Encapsulation** | `Auth` class — password is `private`, only accessible via `setCredentials()` |
| **Abstraction** | User sees a simple menu; binary search, memory management, and snapshot logic are hidden |
| **Inheritance** | `Sensor` is a base class with `virtual display()` — ready for TempSensor, PressureSensor etc. |
| **Polymorphism** | Calling `sensors[i]->display()` through a pointer calls the right version per type |

---

## Password Masking

When you type your password, it shows `****` instead of the real characters.

- **Windows** — uses `_getch()` from `<conio.h>` which reads without echo
- **Linux/Mac** — uses `<termios.h>` to disable terminal echo, reads char by char, restores echo after

The correct version is selected automatically at compile time using `#ifdef _WIN32`.

---

## Memory Management

Every sensor is created with `new`:
```cpp
Sensor* s = new Sensor(name, id, value, minT, maxT);
```

Every sensor must be freed with `delete` when removed or when the program exits:
```cpp
for (int i = 0; i < sensors.size(); i++)
    delete sensors[i];
```

Without this, memory leaks — the program uses RAM that never gets returned to the OS.

---

## Future Plans

- GUI dashboard with real-time sensor charts
- Save sensors to a file or database between sessions
- Sensor type inheritance (TempSensor, PressureSensor, MotionSensor)
- Live IoT hardware connection via serial port or MQTT
- Reliability scoring — track how often each sensor triggers alerts
- Admin vs Viewer user roles

---

*"Programming is not about writing code — it is about modelling reality with precision."*

