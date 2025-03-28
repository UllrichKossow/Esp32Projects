#include "RfSwitch.h"

#include <cstring>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "RfSwitch";

static void isr_callback(void *arg) {
    RfSwitch *r = reinterpret_cast<RfSwitch *>(arg);
    r->Interrupt();
}

static void isr_task(void *arg) {
    RfSwitch *r = reinterpret_cast<RfSwitch *>(arg);
    r->RxTask();
}

RfSwitch::RfSwitch() : m_currentState(false), m_tx_pin(GPIO_NUM_4), m_rx_pin(GPIO_NUM_5) {
    gpio_set_direction(m_tx_pin, GPIO_MODE_OUTPUT);
    gpio_set_drive_capability(m_tx_pin, GPIO_DRIVE_CAP_0);
    gpio_set_level(m_tx_pin, 1);
    gpio_intr_disable(m_tx_pin);
}

void RfSwitch::StartSniffing() { setup_gpio(); }

void RfSwitch::Switch(bool on) {
    ESP_LOGI(TAG, "Switch %s", on ? "on" : "off");
    uint32_t d = on ? 0xceebfb9f : 0xceebfb8f;

    send_word(d, 8);
}

void RfSwitch::Interrupt() {
    ioEvent e;
    e.t = esp_timer_get_time();
    e.v = gpio_get_level(m_rx_pin);
    xQueueSendFromISR(m_rxQueue, &e, NULL);
}

void RfSwitch::RxTask() {
    int64_t last_t = 0;
    const size_t LineLength = 1000;
    char *line = new char[LineLength];
    int idx = 0;
    while (true) {
        ioEvent e;
        if (uxQueueSpacesAvailable(m_rxQueue) < 10) {
            ESP_LOGI(TAG, "Queue available = %i", uxQueueSpacesAvailable(m_rxQueue));
        }

        if (xQueueReceive(m_rxQueue, &e, 100 / portTICK_PERIOD_MS)) {
            int64_t duration = e.t - last_t;
            last_t = e.t;
            char ch = '.';
            if ((duration >= 10000)) // && (duration < 11000))
                ch = 'S';
            else if ((duration >= 2600) && (duration < 2800))
                ch = 's';
            else if ((duration >= 1200) && (duration < 1400))
                ch = 'a';
            else if ((duration >= 190) && (duration < 330))
                ch = 'b';
            // printf("RX %lli %lli %lli %c %i\n", e.t, duration, 5*(duration/5), ch, !e.v);

            if (idx < LineLength - 1) {
                if (ch == 'S') {
                    line[idx++] = '\0';
                    uint32_t code;
                    bool ok = decode_sequence(line, code);
                    printf("Code %s %08lx\n", ok ? "ok" : "fail", code);
                    idx = 0;
                }
                line[idx++] = ch;
                line[idx++] = e.v ? '0' : '1';
            }
        }
    }
}

void RfSwitch::setup_gpio() {
    // change gpio intrrupt type for one pin
    gpio_set_direction(m_rx_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(m_rx_pin, GPIO_PULLUP_ONLY);
    gpio_intr_disable(m_rx_pin);
    gpio_set_intr_type(m_rx_pin, GPIO_INTR_ANYEDGE);
    gpio_intr_enable(m_rx_pin);

    m_rxQueue = xQueueCreate(1000, sizeof(ioEvent));
    xTaskCreate(isr_task, "isr_task", 2048, this, 10, &m_rxTask);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(m_rx_pin, isr_callback, this);
}

// b1s0
// b1a0b1b0 b1a0b1b0 b1b0b1a0 b1b0b1a0 b1a0b1b0 b1a0b1b0 b1a0b1b0 b1b0b1a0
// b1a0b1b0 b1a0b1b0 b1a0b1b0 b1b0b1a0 b1a0b1b0 b1b0b1a0 b1a0b1b0 b1a0b1b0
// b1a0b1b0 b1a0b1b0 b1a0b1b0 b1a0b1b0 b1a0b1b0 b1b0b1a0 b1a0b1b0 b1a0b1b0
// b1a0b1b0 b1b0b1a0 b1b0b1a0 b1a0b1b0 b1a0b1b0 b1a0b1b0 b1a0b1b0 b1a0b1b0 b1S0
bool RfSwitch::decode_sequence(const char *line, uint32_t &code) {
    if (strncmp(line, "S0b1s0b1", 8) != 0) {
        return false;
    }
#if 0
    if (strlen(line) != (2 * (4 + 32 * 8 + 4)))
    {
        return false;
    }
#endif
    const char *p = line + 6;

    for (int i = 0; i < 32; ++i) {
        code <<= 1;
        if (p[8 * i + 2] == 'a') {
            code |= 1;
        }
    }

    return true;
}

void RfSwitch::send_pulse(int t_pulse, int t_pause) {
    int64_t t = esp_timer_get_time();
    gpio_set_level(m_tx_pin, 1);
    while (esp_timer_get_time() < t + t_pulse)
        ;

    t = esp_timer_get_time();
    gpio_set_level(m_tx_pin, 0);
    while (esp_timer_get_time() < t + t_pause)
        ;
}

void RfSwitch::send_bit(bool b) {
    if (b) {
        send_pulse(280, 1200);
        send_pulse(280, 280);
    } else {
        send_pulse(280, 280);
        send_pulse(280, 1200);
    }
}

void RfSwitch::send_word(const uint32_t data, int count) {
    while (count-- > 0) {
        uint32_t out = data;
        send_pulse(280, 2800);
        for (int i = 0; i < 32; ++i) {
            send_bit(out & 0x80000000);
            out <<= 1;
        }
        send_pulse(280, 10000);
    }
}
