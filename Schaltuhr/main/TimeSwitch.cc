#include "TimeSwitch.h"

#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "TimeSwitch";

TimeSwitch::TimeSwitch()
    : m_currentState(bulb_off)
{
    setenv("TZ", "UTC-1", 1);
    tzset();
    Switch(false);
}

void TimeSwitch::Switch(bool state)
{
    m_rfSwitch.Switch(state);
}

void TimeSwitch::ProcessProgramm()
{
    time_t now;
    tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    bulb_state_t calculated_state = bulb_off;
    if ((timeinfo.tm_hour >= 8) && (timeinfo.tm_hour < 12))
    {
        calculated_state = bulb_on_cold;
    }
    else if ((timeinfo.tm_hour >= 12) && (timeinfo.tm_hour < 15))
    {
        calculated_state = bulb_on_warm;
    }
    else if ((timeinfo.tm_hour >= 15) && (timeinfo.tm_hour < 20))
    {
        calculated_state = bulb_on_cold;
    }

    if (calculated_state != m_currentState)
    {
        ESP_LOGI(TAG, "%02i:%02i:%02i", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        m_currentState = calculated_state;
        switch (m_currentState)
        {
        case bulb_off:
            Switch(false);
            break;
        case bulb_on_cold:
        {
            Switch(false);
            vTaskDelay(10000 / portTICK_PERIOD_MS);

            Switch(true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(true);
            break;
        }
        case bulb_on_warm:
        {
            Switch(false);
            vTaskDelay(10000 / portTICK_PERIOD_MS);

            Switch(true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(true);
            break;
        }
        }
    }
}
