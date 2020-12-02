// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <cstdio>
#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp32/clk.h"

#include "RfSwitch.h"
#include "TimeSwitch.h"

static const char* TAG = "app_main";

extern void sync_time();

void light_sleep_enable(void)
{
    int cur_freq_mhz = esp_clk_cpu_freq() / 1000000;
    int xtal_freq = (int) rtc_clk_xtal_freq_get();

    const esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = cur_freq_mhz,
        .min_freq_mhz = xtal_freq,
        .light_sleep_enable = true
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config));
}

void init()
{
    esp_timer_init();
    sync_time();    
}

void timer_cb_slow(void *arg)
{
    TimeSwitch *s = reinterpret_cast<TimeSwitch*>(arg);
    s->ProcessProgramm();
}

void timer_cb_fast(void *arg)
{
    timespec now;
    tm tm_info;
    clock_gettime(CLOCK_REALTIME, &now);
    localtime_r(&now.tv_sec, &tm_info);
    char line[64];
    strftime(line, 64, "%T", &tm_info); 
    ESP_LOGI(TAG, "%s %li", line, now.tv_nsec);
}

void loop()
{
#if 0
    RfSwitch r;
    r.Switch(false);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    r.StartSniffing();
#endif
    TimeSwitch s;
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    esp_timer_init();
    esp_timer_create_args_t create_args_slow = {
        .callback = timer_cb_slow,
        .arg = &s,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "TimerSlow"
    };
    
    esp_timer_handle_t timer_slow;
    esp_timer_create(&create_args_slow, &timer_slow);
    esp_timer_start_periodic(timer_slow, 10*1000*1000);

    esp_timer_create_args_t create_args_fast = {
        .callback = timer_cb_fast,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "TimerFast"
    };

    esp_timer_handle_t timer_fast;
    esp_timer_create(&create_args_fast, &timer_fast);

    // Sync for seconds change
    timespec now, later;
    clock_gettime(CLOCK_REALTIME, &now);
    do
    {
        clock_gettime(CLOCK_REALTIME, &later);
    } while (now.tv_sec == later.tv_sec);

    esp_timer_start_periodic(timer_fast, 1*1000*1000);
    
    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void);
void app_main(void)
{
    light_sleep_enable();
    init();
    while (true)
    {
        loop();
    }
}
