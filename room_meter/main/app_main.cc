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


void init()
{
    sh1106_init();
    sh1106_display_clear();
    sh1106_print_line(0, "Sync time...");
    sync_time();
    sh1106_print_line(0, "Sync time done.");
}


//------------------------------------------------------------------------------------------
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


//------------------------------------------------------------------------------------------
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
    int last_y = 63 - (int) (63 * (values[0] - v_min) / dy);

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


//------------------------------------------------------------------------------------------
void showStatistics(const string &title, const vector<double> values)
{
    double v_max = values[0];
    double v_min = values[0];
    double v_sum = 0;
    for (int i = 0; i < values.size(); ++i)
    {
        v_sum += values[i];
        v_max = max(v_max, values[i]);
        v_min = min(v_min, values[i]);
    }
    sh1106_display_clear();
    int line = 0;
    sh1106_print_line(line++, title.c_str());
    ostringstream s;
    s << "Max: " << v_max;
    sh1106_print_line(line++, s.str().c_str());
    s.str("");

    s << "Min: " << v_min;
    sh1106_print_line(line++, s.str().c_str());
    s.str("");

    s << "Avg: " << v_sum / values.size();
    sh1106_print_line(line++, s.str().c_str());
    s.str("");
}


//------------------------------------------------------------------------------------------
void showSummary(size_t values, size_t cycles, time_t duration)
{
    ostringstream s;
    int line = 0;
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

    sh1106_print_line(line++,str_date);
    sh1106_print_line(line++,s.str().c_str());

    s.str("");
    s << "M " << now_mo.tv_sec << " " << now_mo.tv_nsec/1000000;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Values: " << values;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Cycles: " << cycles;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Duration: " << duration;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Heap: " << xPortGetFreeHeapSize();
    sh1106_print_line(line++, s.str().c_str());
}


//------------------------------------------------------------------------------------------
void loop()
{
    Bme280Controller b;
    b.init();
    b.start();
    int cycle = 0;

    while (true)
    {
        sleepUpTo(5 * 1000 * 1000);
        if (b.getCounter() < 5)
        {
            showSummary(0, 0, 0);
            continue;
        }
        timespec d = b.getDuration();
        vector<measure_t> m = b.getValuesForDuration(128, min(d.tv_sec, 86400L));
        vector<double> plotData;

        switch (cycle)
        {
        case 0:
            showSummary(m.size(), b.getCounter(), d.tv_sec);
            break;
        case 1:
            transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::temp));
            showStatistics("Temperature", plotData);
            break;
        case 2:
            transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::temp));
            plotValues(plotData);
            break;
        case 3:
            transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::hum));
            showStatistics("Humidity", plotData);
            break;
        case 4:
            transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::hum));
            plotValues(plotData);
            break;
        case 5:
            transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::p_nn));
            showStatistics("Pressure", plotData);
            break;
        case 6:
            transform(m.begin(), m.end(), back_inserter(plotData), mem_fn(&measure_t::p_nn));
            plotValues(plotData);
            break;
        }
        ++cycle;
        if (cycle > 6)
        {
            cycle = 0;
        }
    }
}


//------------------------------------------------------------------------------------------
extern "C" void app_main(void);
void app_main()
{
    init();
    while (true)
    {
        loop();
    }
}

