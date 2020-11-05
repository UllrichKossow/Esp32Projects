#ifndef RFSWITCH_H
#define RFSWITCH_H

#include <vector>
#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


class RfSwitch
{
public:
    RfSwitch();

    void StartSniffing();
    void DumpEdges();
    void Clear();

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
    std::vector<ioEvent> m_buffer;

    TaskHandle_t m_rxTask;
    xQueueHandle m_rxQueue;
};

#endif // RFSWITCH_H
