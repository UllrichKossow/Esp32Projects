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
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
}

void init()
{
    esp_timer_init();
    sync_time();
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


    int n = 0;
    while (true)
    {
        ++n;

        timespec now_rt;
        clock_gettime(CLOCK_REALTIME, &now_rt);
        ESP_LOGI(TAG, "t=%li %li", now_rt.tv_sec, now_rt.tv_nsec);

        vTaskDelay(((now_rt.tv_nsec > 10000000) ? 999 : 1000) / portTICK_PERIOD_MS);

        s.ProcessProgramm();
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
