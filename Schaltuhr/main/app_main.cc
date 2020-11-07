// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <cstdio>
#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

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

    while (true)
    {
        timespec now_rt;
        clock_gettime(CLOCK_REALTIME, &now_rt);
        ESP_LOGI(TAG, "t=%li %li", now_rt.tv_sec, now_rt.tv_nsec);

        vTaskDelay(((now_rt.tv_nsec > 10000000) ? 999 : 1000) / portTICK_PERIOD_MS);
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
