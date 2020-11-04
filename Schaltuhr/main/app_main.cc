// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <cstdio>
#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "Schaltuhr.h"

static const char* TAG = "app_main";


extern void sync_time();

void init()
{
    sync_time();
}

void loop()
{
    while (true)
    {
        timespec now_rt;
        clock_gettime(CLOCK_REALTIME, &now_rt);
        ESP_LOGI(TAG, "t2=%li %li", now_rt.tv_sec, now_rt.tv_nsec);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
