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
                    std::string code;
                    bool ok = decode_sequence(line, code);
                    printf("Code %s %s %s\n", line, ok ? "ok" : "fail", code.c_str());
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


bool RfSwitch::decode_sequence(const char *line, std::string &code)
{
    if (strncmp(line, "S0b1s0b1", 8) != 0)
    {
        return false;
    }
    code.append("S");
    const char *p = line + 8;
    int bits = 0;
    while ((p != NULL) && (strlen(p) >= 2))
    {
        if (*p == '.')
        {
            return false;
        }
        char *pos = strstr(p + 2, "a0");
        int len = 0;
        if (pos)
        {
            len = pos - p;
        } else
        {
            len = strlen(p);
        }
        p = pos;
        int pulse = (len - 2) / 2;
        switch (pulse)
        {
        case 1:
            code.append("1");
            break;
        case 3:
            code.append("3");
            break;
        case 5:
            code.append("5");
            break;
        default:
            code.append(".");
            break;
        }

    }
    return true;
}


void RfSwitch::send_pulse(int t_pulse, int t_pause, int count)
{
    while (count-- > 0)
    {
        int64_t t = esp_timer_get_time();
        gpio_set_level(GPIO_NUM_4, 0);
        while (esp_timer_get_time() < t + t_pulse)
            ;

        t = esp_timer_get_time();
        gpio_set_level(GPIO_NUM_4, 1);
        while (esp_timer_get_time() < t + t_pulse)
            ;
    }
}


void RfSwitch::send_pattern(const std::string &pattern, int count)
{
    while (count-- > 0)
    {
        for (size_t i = 0; i < pattern.length(); ++i)
        {
            switch (pattern[i])
            {
            case 'S':
                send_pulse(10000, 300);
                send_pulse(2700, 300);
                break;
            case '1':
                send_pulse(1200, 300);
                send_pulse(300, 300, 1);
                break;
            case '3':
                send_pulse(1200, 300);
                send_pulse(300, 300, 3);
                break;
            case '5':
                send_pulse(1200, 300);
                send_pulse(300, 300, 5);
                break;
            }
        }
    }
}
