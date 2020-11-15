#ifndef RFSWITCH_H
#define RFSWITCH_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <string>

class RfSwitch
{
public:
    RfSwitch();

    void StartSniffing();

    void Interrupt();
    void RxTask();

private:
    void setup_gpio();
    bool decode_sequence(const char *line, std::string &code);

    void send_pulse(int t_pulse, int t_pause, int count = 1);
    void send_pattern(const std::string &pattern, int count = 1);

    struct ioEvent
    {
        int64_t t;
        int v;
    };

private:
    TaskHandle_t m_rxTask;
    xQueueHandle m_rxQueue;
};

#endif // RFSWITCH_H
