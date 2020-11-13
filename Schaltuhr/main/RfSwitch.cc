#include "RfSwitch.h"

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
    char *line = new char[500];
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
            if ((duration >= 10000) && (duration < 11000))
                ch = 'S';
            else if ((duration >= 2600) && (duration < 2800))
                ch = 's';
            else if ((duration >= 1200) && (duration < 1400))
                ch = 'a';
            else if ((duration >= 190) && (duration < 205))
                ch = 'b';
            else if ((duration >= 220) && (duration < 250))
                ch = 'c';
            else if ((duration >= 250) && (duration < 300))
                ch = 'd';
            else if ((duration >= 300) && (duration < 330))
                ch = 'd';
            printf("RX %lli %lli %lli %c %i\n", e.t, duration, 5*(duration/5), ch, !e.v);

            if (idx < 499)
            {
                if (ch == 'S')
                {
                    line[idx++] = '\0';
                    if (idx > 1)
                    {
                        printf("Code %s\n", line);
                        idx = 0;
                    }
                }
                line[idx++] = ch;
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
