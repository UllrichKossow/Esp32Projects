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
    bool m_currentState;

    RfSwitch m_rfSwitch;
};

#endif // TimeSwitch_H
