#ifndef RFSWITCH_H
#define RFSWITCH_H

#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


class RfSwitch
{
public:
    RfSwitch();

    void StartSniffing();

    void Interrupt();
    void RxTask();

private:
    void setup_gpio();

    struct ioEvent
    {
        timespec t;
        int v;
    };

private:
    TaskHandle_t m_rxTask;
    xQueueHandle m_rxQueue;
};

#endif // RFSWITCH_H
