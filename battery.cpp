/*
 Copyright (c) 2010 Mats Rauhala <mats.rauhala@gmail.com>

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/

/* 02.02 2009
 * Mats Rauhala
 *
 * Battery status bar. Reads battery status information and prints colored
 * unicode triangles representing battery status. In this implementation the
 * color codes are for zsh.
 *
 * Made in C++ instead of for example python because I wanted more speed
 * (rendered often) and wanted more experience with C++.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <cstdlib>

using namespace std;

struct BATTERY {
    int designCapacity;
    int warningCapacity;
    int currentCapacity;
};

typedef struct BATTERY battery_t;

const string BAT0("/proc/acpi/battery/BAT0");
const string BAT1("/proc/acpi/battery/BAT1");
const string AC("/proc/acpi/ac_adapter/ACAD/state");
const string RED("%{\033[0;31m%}");
const string GREEN("%{\033[0;32m%}");
const string YELLOW("%{\033[0;33m%}");
const string NOCOLOR("%{\033[0;0m%}");
const string ACSYMBOL(GREEN + "↑" + NOCOLOR);
const string BATTERYSYMBOL(RED + "↓" + NOCOLOR);
const string BARSYMBOL("▶");
const int BARS = 10;

inline bool fileExists(string filename)
{
    struct stat statbuffer;
    int status;
    
    status = stat(filename.c_str(), &statbuffer);

    return status==0?true:false;
}

inline bool acPower(string filename)
{
    string state;
    ifstream file(filename.c_str());

    file >> state;
    file >> state;

    file.close();

    if(state == string("off-line"))
        return false;
    else
        return true;
}

// Get the design capacity
inline void batteryCapacity(string dir, battery_t &battery)
{
    string filename(dir + string("/info"));
    if(!fileExists(filename)) return;
    ifstream file(filename.c_str());
    string temp;
    int state = 0;

    while(!file.eof())
    {
        file >> temp;
        if(temp == "design")
            state = 1;
        else if(state == 1 && temp == "capacity:")
        {
            if(file.eof()) break;
            file >> temp;
            battery.designCapacity = atoi(temp.c_str());
            state = 0;
        }
        else if(state == 1 && temp == "capacity")
            state = 2;
        else if(state == 2 && temp == "warning:")
        {
            if(file.eof()) break;
            file >> temp;
            battery.warningCapacity = atoi(temp.c_str());
            state = 0;
            break;
        }
        else
            state = 0;
    }

    file.close();
}

inline void currentCapacity(string dir, battery_t &battery)
{
    string filename(dir + string("/state")), temp;
    if(!fileExists(filename)) return;
    ifstream file(filename.c_str());
    int state = 0;

    while(!file.eof())
    {
        file >> temp;
        if(state == 0 && temp == "remaining")
            state = 1;
        else if(state == 1 && temp == "capacity:")
        {
            if(file.eof()) break;
            file >> temp;
            battery.currentCapacity = atoi(temp.c_str());
            break;
        }
    }

    file.close();
}

inline int green(battery_t &battery)
{
    return (battery.currentCapacity /
        static_cast<float>(battery.designCapacity)) * BARS;
}

inline void formatBars(int capacity, battery_t &battery)
{
    for(int i = 0; i < BARS; i++)
    {
        if(i < capacity)
            cout << GREEN;
        else if(battery.currentCapacity < battery.warningCapacity)
            cout << RED;
        else
            cout << YELLOW;
        cout << BARSYMBOL;
    }
    cout << NOCOLOR;
}

int main(void)
{
    string batterydir("");
    battery_t battery = {0, 0, 0};
    // Check whether we are on AC power and print an upwards arrow to symbolize
    // AC current
    if(fileExists(AC))
    {
        if(acPower(AC))
            cout << ACSYMBOL;
        else
            cout << BATTERYSYMBOL;
        cout << " ";
    }
    if(fileExists(BAT0))
        batterydir = BAT0;
    else if(fileExists(BAT1))
        batterydir = BAT1;
    // We have no batteries o.O. Exit with return code 0
    if(batterydir == string(""))
        return 0;

    batteryCapacity(batterydir, battery);
    currentCapacity(batterydir, battery);

    formatBars(green(battery), battery);

    return 0;
}
