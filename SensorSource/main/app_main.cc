// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <algorithm>
#include <functional>
#include <sstream>

#include "Bme280Controller.h"
#include "sh1106.h"
#include "SetSystemTime.h"
#include "time_helper.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#include "esp_pm.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

using namespace std;


static const char* TAG = "app_main";


void init_touch()
{

}
//------------------------------------------------------------------------------------------
void init()
{
    sh1106_init();
    sh1106_display_clear();
    sh1106_print_line(0, "Sync time...");
    sync_time(true);
    sh1106_print_line(0, "Sync time done.");
    init_touch();
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
    last_y = min(63, last_y);
    last_y = max(0, last_y);

    for (int x = 0; x < 128; ++x)
    {
        int y = 63 - (int) (63 * (values[x] - v_min) / dy);

        y = min(63, y);
        y = max(0, y);
        ESP_LOGD(TAG, "x=%i, y=%i", x, y);

        sh1106_line(&fb, last_x, min(63,last_y), x, min(63, y));
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
void showSummary(size_t values, size_t cycles, const timespec &duration, uint32_t interval)
{
    ostringstream s;
    struct timespec now_rt, now_mo;
    clock_gettime(CLOCK_REALTIME, &now_rt);
    clock_gettime(CLOCK_MONOTONIC, &now_mo);
    //ESP_LOGI(TAG, "t2=%li %li", now_rt.tv_sec, now_rt.tv_nsec);
    struct tm timeinfo;
    setenv("TZ", "UTC", 1);
    tzset();
    localtime_r(&now_rt.tv_sec, &timeinfo);
    char str_time[64];
    char str_date[64];
    strftime(str_time, sizeof(str_time), "%T", &timeinfo);
    strftime(str_date, sizeof(str_date), "%F", &timeinfo);
    s << str_time << " " << now_rt.tv_nsec/1000000 ;

    int line = 0;
    sh1106_display_clear();
    sh1106_print_line(line++,str_date);
    sh1106_print_line(line++,s.str().c_str());

    s.str("");
    s << "Mo " << now_mo.tv_sec << " " << now_mo.tv_nsec/1000000;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Nu " << values;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Cy " << cycles;
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "Du " << timespec_to_string(duration);
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "In " << interval/1000000 << " sec.";
    sh1106_print_line(line++, s.str().c_str());

    s.str("");
    s << "He " << xPortGetFreeHeapSize()/1024 << " kByte." ;
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
        //sleepUpTo(5 * 1000 * 1000);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        if (b.getCounter() < 5)
        {
            showSummary(0, 0, {0,0}, 0);
            continue;
        }
        timespec d = b.getDuration();
        vector<measure_t> m = b.getValuesForDuration(128, min(d.tv_sec, (time_t)86400U));
        vector<double> plotData;

        switch (cycle)
        {
        case 0:
            showSummary(b.getNumberOfValues(), b.getCounter(), d, b.getCurrentInterval());
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


void light_sleep_enable(void)
{
    esp_pm_config_t pm_config;
    esp_pm_get_configuration(&pm_config);
    pm_config.light_sleep_enable = true;
    
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
}

//------------------------------------------------------------------------------------------
extern "C" void app_main(void);
void app_main()
{
    //light_sleep_enable();
    init();
    while (true)
    {
        loop();
    }
}

