#ifndef TimeSwitch_H
#define TimeSwitch_H

#include "RfSwitch.h"

#include <ctime>

class cJSON;

class TimeSwitch
{
public:
    TimeSwitch();

    void ProcessProgramm();

private:
    void Switch(bool state);
    bool inRange(tm &t, tm &start, tm &stop);
    bool readTable(const char *json);

private:
    enum bulb_state_t
    {
        bulb_unknown,
        bulb_off,
        bulb_on_4k0,
        bulb_on_2k7,
        bulb_on_6k5
    };
    bulb_state_t m_currentState;
    cJSON *m_table;

    RfSwitch m_rfSwitch;
};

#endif // TimeSwitch_H
