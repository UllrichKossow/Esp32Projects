// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <algorithm>
#include <sstream>

#include "Bme280Controller.h"
#include "bme280_access.h"
#include "sh1106.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

using namespace std;


static const char* TAG = "app_main";

void sync_time(void);


void show_date_time()
{
    ostringstream s;
    struct timespec now_rt, now_mo;
    clock_gettime(CLOCK_REALTIME, &now_rt);
    clock_gettime(CLOCK_MONOTONIC, &now_mo);
    //ESP_LOGI(TAG, "t2=%li %li", now_rt.tv_sec, now_rt.tv_nsec);
    struct tm timeinfo;
    setenv("TZ", "UTC-2", 1);
    tzset();
    localtime_r(&now_rt.tv_sec, &timeinfo);
    char str_time[64];
    char str_date[64];
    strftime(str_time, sizeof(str_time), "%T", &timeinfo);
    strftime(str_date, sizeof(str_date), "%F", &timeinfo);
    s << str_time << " " << now_rt.tv_nsec/1000000 ;

    sh1106_print_line(2,str_date);
    sh1106_print_line(3,s.str().c_str());

    s.str("");
    s << now_mo.tv_sec << " " << now_mo.tv_nsec/1000000 << " " << now_mo.tv_sec % 3600;
    sh1106_print_line(4, s.str().c_str());
}

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
    uint64_t delay_us = min(maxSleep, usec);
    ESP_LOGD(TAG, "Lightsleep for %lli us", delay_us);

    esp_sleep_enable_timer_wakeup(delay_us);
    esp_light_sleep_start();
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
        sleepUpTo(10*1000*1000);
        uint32_t cnt = b.getCounter();
        while (cnt == b.getCounter())
        {
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
        timespec d = b.getDuration();
        ESP_LOGD(TAG, "Duration = %li, %li", d.tv_sec, d.tv_nsec);
        ostringstream s;
        s << cnt << " " << d.tv_sec + d.tv_nsec / 1000000000.0;
        sh1106_print_line(0, s.str().c_str());

        s.str("");
        s << "n: " << b.getNumberOfValues();
        sh1106_print_line(1, s.str().c_str());
        show_date_time();
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

