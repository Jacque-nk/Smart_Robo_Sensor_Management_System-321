#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>

#ifdef _WIN32
  #include <conio.h>   // Windows: _getch()
#else
  #include <termios.h> // Linux/Mac: manual echo off
  #include <unistd.h>
#endif

using namespace std;

// ============================================================
// SENSOR CLASS
// ============================================================
class Sensor
{
public:
    string name;
    int    id;
    float  value;
    float  minT, maxT;

    Sensor(string n, int i, float v, float mn, float mx)
        : name(n), id(i), value(v), minT(mn), maxT(mx) {}

    virtual void display()
    {
        cout << "\n---------------------------";
        cout << "\nName  : " << name;
        cout << "\nID    : " << id;
        cout << "\nValue : " << value;
        cout << "\nRange : " << minT << " - " << maxT;
        if (value < minT || value > maxT)
            cout << "\n*** ALERT: Value out of threshold! ***";
        else
            cout << "\nStatus: OK";
        cout << "\n---------------------------";
    }
};

// ============================================================
// ACTION RECORD  (what the undo stack stores)
// ============================================================
// Every time you do something, we save:
//   - what TYPE of action it was  (ADD / UPDATE / DELETE)
//   - a snapshot of the sensor BEFORE the change
//   - the sensor pointer itself
// ============================================================
enum ActionType { ADD, UPDATE, DELETE_SENSOR };

struct Action
{
    ActionType type;
    Sensor*    sensor;      // the live sensor pointer
    Sensor     snapshot;    // copy of sensor BEFORE the change

    Action(ActionType t, Sensor* s)
        : type(t), sensor(s), snapshot(*s) {}
};

// ============================================================
// GLOBAL DATA STRUCTURES
// ============================================================
vector<Sensor*> sensors;        // all sensors
queue<Sensor*>  history;        // queue — records add order (history)
stack<Action>   undoStack;      // undo stack — stores Action records

// ============================================================
// HIDDEN PASSWORD INPUT
// Reads characters one by one without showing them on screen
// Shows * for each character typed, handles backspace too
// ============================================================
string getPassword()
{
    string pass = "";
    char ch;

#ifdef _WIN32
    // Windows — _getch() reads one char at a time, no echo at all
    while (true)
    {
        ch = _getch();
        if (ch == '\r' || ch == '\n') break;  // Enter pressed

        if ((ch == '\b' || ch == 127) && !pass.empty())  // backspace
        {
            cout << "\b \b" << flush;
            pass.pop_back();
        }
        else if (ch != '\b' && ch != 127)
        {
            pass += ch;
            cout << '*' << flush;   // print * immediately
        }
    }
#else
    // Linux / Mac — disable terminal echo so characters don't show
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);  // turn off echo AND line buffering
    newt.c_cc[VMIN]  = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (true)
    {
        ch = getchar();
        if (ch == '\n' || ch == '\r') break;  // Enter pressed

        if ((ch == 127 || ch == '\b') && !pass.empty())  // backspace
        {
            cout << "\b \b" << flush;
            pass.pop_back();
        }
        else if (ch != 127 && ch != '\b')
        {
            pass += ch;
            cout << '*' << flush;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // restore terminal echo
#endif

    cout << "\n";
    return pass;
}

// ============================================================
// AUTH CLASS — password is private (encapsulated)
// ============================================================
// username is public  — okay to see
// password is private — only this class can touch it
// ============================================================
class Auth
{
public:
    string username;

private:
    string password;   // PRIVATE — nothing outside this class can read/set it directly

public:
    // Set credentials (the only way to give password to this class)
    void setCredentials(string u, string p)
    {
        username = u;
        password = p;
    }

    // Check against file
    bool checkUser()
    {
        ifstream file("users.txt");
        string u, p;
        while (file >> u >> p)
            if (u == username && p == password) return true;
        return false;
    }

    // Save new account to file
    void saveToFile()
    {
        ofstream file("users.txt", ios::app);
        file << username << " " << password << "\n";
    }
};

bool login()
{
    Auth auth;
    string u, p;

    for (int i = 0; i < 3; i++)
    {
        cout << "Username: "; cin >> u;
        cout << "Password: "; p = getPassword();

        auth.setCredentials(u, p);   // pass both at once — only way in

        if (auth.checkUser()) { cout << "Login successful!\n"; return true; }
        cout << "Wrong credentials. Attempts left: " << (2 - i) << "\n";
    }

    char c;
    cout << "Create new account? (y/n): "; cin >> c;
    if (c == 'y')
    {
        cout << "New username: "; cin >> u;
        cout << "New password: "; p = getPassword();
        auth.setCredentials(u, p);
        auth.saveToFile();
        cout << "Account created!\n";
        return true;
    }
    return false;
}

// ============================================================
// STEP-BY-STEP INPUT WITH BACK SUPPORT
// Lets user go back to a previous field if they made a mistake
// ============================================================
bool inputSensorFields(string &name, int &id, float &value, float &minT, float &maxT)
{
    int step = 0;
    string input;

    while (step < 5)
    {
        if      (step == 0) cout << "\nName          (or 'b' back, 'q' quit): ";
        else if (step == 1) cout << "ID            (or 'b' back, 'q' quit): ";
        else if (step == 2) cout << "Value         (or 'b' back, 'q' quit): ";
        else if (step == 3) cout << "Min threshold (or 'b' back, 'q' quit): ";
        else if (step == 4) cout << "Max threshold (or 'b' back, 'q' quit): ";

        cin >> input;

        if (input == "q") return false;

        if (input == "b")
        {
            if (step == 0) { cout << "Already at first field.\n"; continue; }
            step--;
            cout << "(Went back one step)\n";
            continue;
        }

        if (step == 0) { name = input; step++; }
        else if (step == 1)
        {
            try { id = stoi(input); step++; }
            catch (...) { cout << "ID must be a whole number. Try again.\n"; }
        }
        else if (step == 2)
        {
            try { value = stof(input); step++; }
            catch (...) { cout << "Value must be a number. Try again.\n"; }
        }
        else if (step == 3)
        {
            try { minT = stof(input); step++; }
            catch (...) { cout << "Min threshold must be a number. Try again.\n"; }
        }
        else if (step == 4)
        {
            try { maxT = stof(input); step++; }
            catch (...) { cout << "Max threshold must be a number. Try again.\n"; }
        }
    }
    return true;
}

// ============================================================
// ADD SENSORS
// ============================================================
void addSensors()
{
    int count;
    cout << "\nHow many sensors do you want to add? ";
    cin >> count;

    for (int i = 0; i < count; i++)
    {
        cout << "\n--- Sensor " << (i + 1) << " ---";
        cout << "\n(type 'b' at any field to go back, 'q' to cancel this sensor)\n";

        string name; int id; float value, minT, maxT;

        if (!inputSensorFields(name, id, value, minT, maxT))
        {
            cout << "Sensor entry cancelled.\n";
            continue;
        }

        Sensor* s = new Sensor(name, id, value, minT, maxT);

        sensors.push_back(s);
        history.push(s);
        undoStack.push(Action(ADD, s));

        cout << "Sensor added!\n";
    }
}

// ============================================================
// DISPLAY ALL SENSORS
// ============================================================
void showSensors()
{
    if (sensors.empty()) { cout << "\nNo sensors found.\n"; return; }
    cout << "\n===== ALL SENSORS =====";
    for (int i = 0; i < (int)sensors.size(); i++)
        sensors[i]->display();
}

// ============================================================
// SEARCH SENSOR BY ID
// ============================================================
bool sortById(Sensor* a, Sensor* b) { return a->id < b->id; }

void searchSensor()
{
    if (sensors.empty()) { cout << "\nNo sensors to search.\n"; return; }

    int target;
    cout << "\nEnter Sensor ID to search: "; cin >> target;

    sort(sensors.begin(), sensors.end(), sortById);

    int lo = 0, hi = (int)sensors.size() - 1;
    bool found = false;

    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;
        if (sensors[mid]->id == target)
        {
            cout << "\nSensor found!";
            sensors[mid]->display();
            found = true; break;
        }
        else if (sensors[mid]->id < target) lo = mid + 1;
        else                                hi = mid - 1;
    }

    if (!found) cout << "\nSensor ID " << target << " not found.\n";
}

// ============================================================
// DELETE SENSOR BY ID
// ============================================================
void deleteSensor()
{
    if (sensors.empty()) { cout << "\nNo sensors to delete.\n"; return; }

    int target;
    cout << "\nEnter Sensor ID to delete: "; cin >> target;

    for (int i = 0; i < (int)sensors.size(); i++)
    {
        if (sensors[i]->id == target)
        {
            // Save BEFORE removing — undo needs the object alive
            undoStack.push(Action(DELETE_SENSOR, sensors[i]));
            cout << "Deleted sensor: " << sensors[i]->name << "\n";
            sensors.erase(sensors.begin() + i);
            return;
        }
    }
    cout << "Sensor ID " << target << " not found.\n";
}

// ============================================================
// UPDATE SENSOR VALUE
// ============================================================
void updateSensor()
{
    if (sensors.empty()) { cout << "\nNo sensors to update.\n"; return; }

    int target;
    cout << "\nEnter Sensor ID to update: "; cin >> target;

    for (int i = 0; i < (int)sensors.size(); i++)
    {
        if (sensors[i]->id == target)
        {
            // Save old state BEFORE changing — undo will restore this
            undoStack.push(Action(UPDATE, sensors[i]));

            float newVal;
            cout << "Current value: " << sensors[i]->value << "\n";
            cout << "New value    : "; cin >> newVal;
            sensors[i]->value = newVal;

            cout << "Updated!\n";
            sensors[i]->display();
            return;
        }
    }
    cout << "Sensor ID " << target << " not found.\n";
}

// ============================================================
// UNDO — REVERSES THE LAST ACTION EXACTLY
//
// ADD         → removes the sensor that was just added
// UPDATE      → restores the old value before the update
// DELETE      → puts the sensor back into the vector
// ============================================================
void undo()
{
    if (undoStack.empty()) { cout << "\nNothing to undo.\n"; return; }

    Action last = undoStack.top();
    undoStack.pop();

    if (last.type == ADD)
    {
        for (int i = (int)sensors.size() - 1; i >= 0; i--)
        {
            if (sensors[i] == last.sensor)
            {
                sensors.erase(sensors.begin() + i);
                break;
            }
        }
        cout << "Undo: removed added sensor '" << last.sensor->name << "'\n";
        delete last.sensor;
    }
    else if (last.type == UPDATE)
    {
        // Restore old value from snapshot
        float oldVal = last.snapshot.value;
        last.sensor->value = oldVal;
        cout << "Undo: value restored to " << oldVal << "\n";
        last.sensor->display();
    }
    else if (last.type == DELETE_SENSOR)
    {
        sensors.push_back(last.sensor);
        cout << "Undo: restored deleted sensor '" << last.sensor->name << "'\n";
        last.sensor->display();
    }
}

// ============================================================
// HISTORY (queue — shows sensors in the order they were added)
// ============================================================
void showHistory()
{
    if (history.empty()) { cout << "\nHistory is empty.\n"; return; }

    cout << "\n===== SENSOR HISTORY (order added) =====";
    queue<Sensor*> temp = history;
    int count = 1;
    while (!temp.empty())
    {
        cout << "\n[" << count++ << "] " << temp.front()->name;
        temp.pop();
    }
    cout << "\n";
}

// ============================================================
// MENU
// ============================================================
void menu()
{
    cout << "\n==============================";
    cout << "\n  SMART SENSOR SYSTEM v2";
    cout << "\n==============================";
    cout << "\n1. Add Sensors";
    cout << "\n2. Display All Sensors";
    cout << "\n3. Search Sensor";
    cout << "\n4. Delete Sensor";
    cout << "\n5. Update Sensor Value";
    cout << "\n6. History";
    cout << "\n7. Undo Last Action";
    cout << "\n8. Exit";
    cout << "\nChoice: ";
}

// ============================================================
// MAIN
// ============================================================
int main()
{
    cout << "====== SMART SENSOR SYSTEM ======\n";

    if (!login()) { cout << "Access denied.\n"; return 0; }

    int choice;
    do
    {
        menu();
        cin >> choice;

        switch (choice)
        {
        case 1: addSensors();    break;
        case 2: showSensors();   break;
        case 3: searchSensor();  break;
        case 4: deleteSensor();  break;
        case 5: updateSensor();  break;
        case 6: showHistory();   break;
        case 7: undo();          break;
        case 8: cout << "Goodbye!\n"; break;
        default: cout << "Invalid choice.\n";
        }

    } while (choice != 8);

    // Clean up remaining sensors in memory
    for (int i = 0; i < (int)sensors.size(); i++)
        delete sensors[i];

    return 0;
}