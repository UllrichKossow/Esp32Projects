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
#include "driver/gpio.h"

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
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
}

void init()
{
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    gpio_set_drive_capability(GPIO_NUM_5, GPIO_DRIVE_CAP_DEFAULT);
    gpio_set_level(GPIO_NUM_5, 0);
    esp_timer_init();
    sync_time();    
    gpio_set_level(GPIO_NUM_5, 1);
}

void timer_callback_switch(void *arg)
{
    TimeSwitch *s = reinterpret_cast<TimeSwitch*>(arg);
    s->ProcessProgramm();
}

void timer_callback_watch(void *arg)
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
    esp_timer_create_args_t create_args_switch = {
        .callback = timer_callback_switch,
        .arg = &s,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "TimeSwitch"
    };
    
    esp_timer_handle_t timer_for_switch;
    esp_timer_create(&create_args_switch, &timer_for_switch);
    esp_timer_start_periodic(timer_for_switch, 10*1000*1000);

    esp_timer_create_args_t create_args_watch = {
        .callback = timer_callback_watch,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "TimeWatch"
    };

    esp_timer_handle_t timer_for_watch;
    esp_timer_create(&create_args_watch, &timer_for_watch);

    // Sync for seconds change
    timespec now, later;
    clock_gettime(CLOCK_MONOTONIC, &now);
    do
    {
	clock_gettime(CLOCK_MONOTONIC, &later);
    } while (now.tv_sec == later.tv_sec);

    //esp_timer_start_periodic(timer_for_watch, 1*1000*1000);
    
    while (true)
    {
        gpio_set_level(GPIO_NUM_5, 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_5, 1);
        vTaskDelay(2800 / portTICK_PERIOD_MS);
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
