#include "RfSwitch.h"

#include "driver/gpio.h"

struct callback_arg
{
    RfSwitch *instance;
    uint32_t pin;
};

static void isr_callback(void *arg)
{
    callback_arg *cb_arg = (callback_arg *) arg;
}


RfSwitch::RfSwitch()
{

}

void RfSwitch::StartSniffing()
{

}

void RfSwitch::DumpEdges()
{

}

void RfSwitch::Clear()
{

}
