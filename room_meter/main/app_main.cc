// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <algorithm>
#include <functional>
#include <sstream>

#include "Bme280Controller.h"
#include "bme280_access.h"
#include "sh1106.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
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

    s.str("");
    s << "Heap: " << xPortGetFreeHeapSize();
    sh1106_print_line(5, s.str().c_str());
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
    if (maxSleep > 10)
        maxSleep -= 10;
    uint64_t delay_us = min(maxSleep, usec);
    //ESP_LOGD(TAG, "Lightsleep for %lli us", delay_us);

    esp_sleep_enable_timer_wakeup(delay_us);
    esp_light_sleep_start();
}


void plotValues(vector<double> values)
{
    double v_max = values[0];
    double v_min = values[0];
    for (int i = 0; i < values.size(); ++i)
    {
        v_max = max(v_max, values[i]);
        v_min = min(v_min, values[i]);
    }
    double dy = v_max - v_min;
    frame_buffer_t fb;
    sh1106_clear_fb(&fb);
    int last_x = 0;
    int last_y = 0;
    for (int x = 0; x < 128; ++x)
    {
        int y = 63 - (int) (63 * (values[x] - v_min) / dy);
        //ESP_LOGD(TAG, "x=%i, y=%i", x, y);

        sh1106_line(&fb, last_x, last_y, x, y);
        last_x = x;
        last_y = y;
    }
    sh1106_write_fb(&fb);

}

void loop()
{
#if 1
    Bme280Controller b;
    b.init();
    b.start();
    uint32_t cnt = b.getCounter();

    while (true)
    {
        ESP_LOGD(TAG, "%s", __FUNCTION__);
        sleepUpTo(10000000);
        if (cnt != b.getCounter())
        {
            ostringstream s;
            cnt = b.getCounter();
            timespec d = b.getDuration();

            if (cnt % 10 == 5)
            {
                s << "n: " << b.getNumberOfValues();
                sh1106_print_line(1, s.str().c_str());
                s.str("");

                ESP_LOGD(TAG, "Duration = %li, %li", d.tv_sec, d.tv_nsec);
                s << cnt << " " << d.tv_sec + d.tv_nsec / 1000000000.0;
                sh1106_print_line(0, s.str().c_str());
            }
            if (cnt % 10 == 0)
            {
                vector<measure_t> m = b.getValuesForDuration(128, min(d.tv_sec, 86400L));

                vector<double> plotData;
                transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::p_nn));
                plotValues(plotData);
                vector<double> x(m.size());
                double v_max = m[0].p_nn;
                double v_min = m[0].p_nn;
                for (int i = 0; i < m.size(); ++i)
                {
                    x[i] = m[i].p_nn;
                    v_max = max(v_max, x[i]);
                    v_min = min(v_min, x[i]);
                }
                s.str("");
                s << v_min << " " << v_max;
                sh1106_print_line(7, s.str().c_str());
            }
        }
        //show_date_time();
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

