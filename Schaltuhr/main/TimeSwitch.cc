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
}

void TimeSwitch::Switch(bool state)
{
    m_rfSwitch.Switch(state);
}

bool TimeSwitch::inRange(tm &t, tm &start, tm &stop)
{
    return (mktime(&t) >= mktime(&start)) && (mktime(&t) < mktime(&stop));
}

void TimeSwitch::ProcessProgramm()
{
    time_t now;
    tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    timeinfo.tm_mday = timeinfo.tm_wday = timeinfo.tm_yday = timeinfo.tm_year = timeinfo.tm_mon = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    tm t8{.tm_hour = 8};
    tm t11{.tm_hour = 11};
    tm t13{.tm_hour = 13};
    tm t14{.tm_hour = 14};
    tm t16{.tm_hour = 16};
    tm t20{.tm_hour = 20};

#pragma GCC diagnostic pop
    bulb_state_t calculated_state = bulb_off;
    if (inRange(timeinfo, t8, t11))
    {
        calculated_state = bulb_on_6k5;
    }
    else if (inRange(timeinfo, t11, t13))
    {
        calculated_state = bulb_on_4k0;
    }
    else if (inRange(timeinfo, t13, t14))
    {
        calculated_state = bulb_on_2k7;
    }
    else if (inRange(timeinfo, t14, t16))
    {
        calculated_state = bulb_on_4k0;
    }
    else if (inRange(timeinfo, t16, t20))
    {
        calculated_state = bulb_on_6k5;
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

        case bulb_on_6k5:
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

        case bulb_on_4k0:
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

        case bulb_on_2k7:
        {
            Switch(false);
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            Switch(true);
            break;
        }
        }
    }
}
