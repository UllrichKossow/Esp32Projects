#include "RfSwitch.h"


#include <cstring>

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

static const char* TAG = "RfSwitch";

static void isr_callback(void *arg)
{
    RfSwitch *r = reinterpret_cast<RfSwitch *>(arg);
    r->Interrupt();
}

static void isr_task(void *arg)
{
    RfSwitch *r = reinterpret_cast<RfSwitch *>(arg);
    r->RxTask();
}


RfSwitch::RfSwitch()
{

}

void RfSwitch::StartSniffing()
{
    setup_gpio();
}

void RfSwitch::Interrupt()
{
    ioEvent e;
    e.t = esp_timer_get_time();
    e.v = gpio_get_level(GPIO_NUM_5);
    xQueueSendFromISR(m_rxQueue, &e, NULL);
}

void RfSwitch::RxTask()
{
    int64_t last_t = 0;
    const size_t LineLength = 1000;
    char *line = new char[LineLength];
    int idx = 0;
    while (true)
    {
        ioEvent e;
        if (uxQueueSpacesAvailable(m_rxQueue) < 10)
        {
            ESP_LOGI(TAG, "Queue available = %i", uxQueueSpacesAvailable(m_rxQueue));
        }

        if (xQueueReceive(m_rxQueue, &e, 100/portTICK_PERIOD_MS ))
        {
            int64_t duration = e.t - last_t;
            last_t = e.t;
            char ch = '.';
            if ((duration >= 10000))// && (duration < 11000))
                ch = 'S';
            else if ((duration >= 2600) && (duration < 2800))
                ch = 's';
            else if ((duration >= 1200) && (duration < 1400))
                ch = 'a';
            else if ((duration >= 190) && (duration < 330))
                ch = 'b';
            printf("RX %lli %lli %lli %c %i\n", e.t, duration, 5*(duration/5), ch, !e.v);

            if (idx < LineLength-1)
            {
                if (ch == 'S')
                {
                    line[idx++] = '\0';
                    uint32_t code;
                    bool ok = decode_sequence(line, code);
                    printf("Code %s %s %i\n", line, ok ? "ok" : "fail", code);
                    idx = 0;
                }
                line[idx++] = ch;
                line[idx++] = e.v ? '0' : '1';
            }
        }
    }
}

void RfSwitch::setup_gpio()
{
    //change gpio intrrupt type for one pin
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_5, GPIO_PULLUP_ONLY);
    gpio_intr_disable(GPIO_NUM_5);
    gpio_set_intr_type(GPIO_NUM_5 , GPIO_INTR_ANYEDGE);
    gpio_intr_enable(GPIO_NUM_5);

    m_rxQueue = xQueueCreate(1000, sizeof(ioEvent));
    xTaskCreate(isr_task, "isr_task", 2048, this, 10, &m_rxTask);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, isr_callback, this);
}

bool RfSwitch::decode_sequence(const char *line, uint32_t &code)
{
    code = 0;
    if (strncmp(line, "S0a1s0a1", 8) != 0)
    {
        return false;
    }
    const char *p = line + 8;
    int bit = 0;
    while (strlen(p) >= 2)
    {
        if (*p == '.')
        {
            return false;
        }
        p += 2;
    }
    return true;
}
