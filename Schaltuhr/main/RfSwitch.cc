#include "RfSwitch.h"

#include "esp_log.h"
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

void RfSwitch::DumpEdges()
{

}

void RfSwitch::Clear()
{

}

void RfSwitch::Interrupt()
{
    ioEvent e;
    clock_gettime(CLOCK_MONOTONIC, &e.t);
    e.v = gpio_get_level(GPIO_NUM_5);
    xQueueSendFromISR(m_rxQueue, &e, NULL);
}

void RfSwitch::RxTask()
{
    while (true)
    {
        ioEvent e;
        if (xQueueReceive(m_rxQueue, &e, portMAX_DELAY))
        {
            m_buffer.push_back(e);
            ESP_LOGI(TAG, "Edge %li %li %i", e.t.tv_sec, e.t.tv_nsec, e.v);
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

    m_rxQueue = xQueueCreate(100, sizeof(ioEvent));
    xTaskCreate(isr_task, "isr_task", 2048, this, 10, &m_rxTask);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, isr_callback, this);
}
