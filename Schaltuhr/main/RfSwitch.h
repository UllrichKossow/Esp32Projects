#ifndef RFSWITCH_H
#define RFSWITCH_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include <string>

class RfSwitch
{
public:
    RfSwitch();

    void StartSniffing();

    void Interrupt();
    void RxTask();

    void Switch(bool on);

private:
    void setup_gpio();
    bool decode_sequence(const char *line, uint32_t &code);

    void send_pulse(int t_pulse, int t_pause);
    void send_bit(bool b);
    void send_word(const uint32_t data, int count = 1);

    struct ioEvent
    {
        int64_t t;
        int v;
    };

private:
    TaskHandle_t m_rxTask;
    xQueueHandle m_rxQueue;
    bool m_currentState;
    gpio_num_t m_tx_pin, m_rx_pin;
};

#endif // RFSWITCH_H
