#ifndef TimeSwitch_H
#define TimeSwitch_H

#include "RfSwitch.h"

class TimeSwitch
{
public:
    TimeSwitch();

    void ProcessProgramm();

private:
    void Switch(bool state);

private:
    enum bulb_state_t { bulb_off, bulb_on_warm, bulb_on_cold};
    bulb_state_t m_currentState;


    RfSwitch m_rfSwitch;
};

#endif // TimeSwitch_H
