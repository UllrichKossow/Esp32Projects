#include "TimeSwitch.h"

#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "TimeSwitch";

TimeSwitch::TimeSwitch()
    : m_currentState(false)
{
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
    setenv("TZ", "UTC-1", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "%02i:%02i:%02i", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    bool on = (timeinfo.tm_hour >= 8) && (timeinfo.tm_hour < 20);
    if (on != m_currentState)
    {
        m_currentState = on;
        if (on)
        {
            Switch(true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Switch(true);
        }
        else
        {
            Switch(false);
        }
    }
}
