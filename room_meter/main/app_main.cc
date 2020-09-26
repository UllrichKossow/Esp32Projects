// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <algorithm>
#include "Bme280Controller.h"
#include "bme280_access.h"
#include "sh1106.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

static const char* TAG = "app_main";

void sync_time(void);

void init()
{
    sh1106_init();
    sh1106_display_clear();
    sh1106_print_line(0, "Sync time...");
    sync_time();
}

void sleepUpTo(uint64_t usec)
{
    uint64_t nextAlarm = esp_timer_get_next_alarm();
    uint64_t maxSleep = nextAlarm - esp_timer_get_time();
    if (maxSleep > 100)
        maxSleep -= 100;
    uint64_t delay_us = std::min(maxSleep, usec);
    ESP_LOGD(TAG, "Lightsleep for %lli us", delay_us);
    esp_sleep_enable_timer_wakeup(delay_us);
    esp_light_sleep_start();
    while (esp_timer_get_next_alarm() == nextAlarm)
    {
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}


void loop()
{
#if 1
    Bme280Controller b;
    b.init();
    b.start();
    while (true)
    {
        ESP_LOGD(TAG, "%s", __FUNCTION__);
        //vTaskDelay(10000/portTICK_PERIOD_MS);
        sleepUpTo(10000000);
        //timespec d = b.getDuration();
        //ESP_LOGD(TAG, "Duration = %li, %li", d.tv_sec, d.tv_nsec);
    }
#else
    read_bme();
#endif
}

extern "C" void app_main(void);
void app_main()
{
    init();
    while (true)
    {
        loop();
    }
}

