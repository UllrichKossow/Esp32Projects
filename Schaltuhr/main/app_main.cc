// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <cstdio>
#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "driver/gpio.h"

#include "RfSwitch.h"
#include "Schaltuhr.h"

static const char* TAG = "app_main";


extern void sync_time();

void init()
{
    esp_timer_init();
    sync_time();
}

void loop()
{
    RfSwitch r;
    r.StartSniffing();

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_drive_capability(GPIO_NUM_4, GPIO_DRIVE_CAP_0 );
    gpio_set_level(GPIO_NUM_4, 1);
    gpio_intr_disable(GPIO_NUM_4);

    int n = 0;
    while (true)
    {
        timespec now_rt;
        clock_gettime(CLOCK_REALTIME, &now_rt);
        ESP_LOGI(TAG, "t=%li %li", now_rt.tv_sec, now_rt.tv_nsec);

        vTaskDelay(((now_rt.tv_nsec > 10000000) ? 999 : 1000) / portTICK_PERIOD_MS);
        if (n < 16)
        {
            gpio_set_level(GPIO_NUM_4, n & 1);
            ++n;
        }
    }
}

extern "C" void app_main(void);
void app_main(void)
{
    init();
    while (true)
    {
        loop();
    }
}
