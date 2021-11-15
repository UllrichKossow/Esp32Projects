#include "TimeSwitch.h"

#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "MqttClient.h"

static const char *TAG = "TimeSwitch";

TimeSwitch::TimeSwitch()
    : m_currentState(bulb_unknown), m_table(nullptr)
{
    setenv("TZ", "UTC-2", 1);
    tzset();
    readTable("{\"cams\":[{\"time\":\"08:00\",\"state\":\"bulb_6k5\"},{\"time\":\"16:00\",\"state\":\"bulb_off\"}]}");
}

void TimeSwitch::Switch(bool state)
{
    m_rfSwitch.Switch(state);
    MqttClient::instance()->publish("Schaltuhr/switch", state ? "on" : "off");
}

bool TimeSwitch::readTable(const char *json)
{
    m_table = cJSON_Parse(json);
    cJSON *p = cJSON_GetObjectItem(m_table, "cams");
    return true;
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
    tm t10{.tm_hour = 10};
    tm t11{.tm_hour = 11};
    tm t12{.tm_hour = 12};
    tm t13{.tm_hour = 13};
    tm t14{.tm_hour = 14};
    tm t16{.tm_hour = 16};
    tm t17{.tm_hour = 17};
    tm t18{.tm_hour = 18};
    tm t20{.tm_hour = 20};

#pragma GCC diagnostic pop
    bulb_state_t calculated_state = bulb_off;
    if (inRange(timeinfo, t8, t10))
    {
        calculated_state = bulb_on_6k5;
    }
    else if (inRange(timeinfo, t10, t12))
    {
        calculated_state = bulb_on_4k0;
    }
    else if (inRange(timeinfo, t12, t16))
    {
        calculated_state = bulb_on_2k7;
    }
    else if (inRange(timeinfo, t16, t18))
    {
        calculated_state = bulb_on_4k0;
    }
    else if (inRange(timeinfo, t18, t20))
    {
        calculated_state = bulb_on_6k5;
    }

    if (calculated_state != m_currentState)
    {
        ESP_LOGI(TAG, "%02i:%02i:%02i", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        char text[80];
        strftime(text, 80, "%T", &timeinfo);
        MqttClient::instance()->publish("Schaltuhr/timestamp", text, true);
        m_currentState = calculated_state;
        const int t_reset = 10000;
        const int t_pulse = 500;
        switch (m_currentState)
        {
        case bulb_off:
            Switch(false);
            MqttClient::instance()->publish("Schaltuhr/state", "bulb_off", true);
            break;

        case bulb_on_6k5:
        {
            Switch(false);
            vTaskDelay(t_reset / portTICK_PERIOD_MS);

            Switch(true);
            vTaskDelay(t_pulse / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(t_pulse / portTICK_PERIOD_MS);
            Switch(true);
            vTaskDelay(t_pulse / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(t_pulse / portTICK_PERIOD_MS);
            Switch(true);
            MqttClient::instance()->publish("Schaltuhr/state", "bulb_on_6k5", true);
            break;
        }

        case bulb_on_4k0:
        {
            Switch(false);
            vTaskDelay(t_reset / portTICK_PERIOD_MS);

            Switch(true);
            vTaskDelay(t_pulse / portTICK_PERIOD_MS);
            Switch(false);
            vTaskDelay(t_pulse / portTICK_PERIOD_MS);
            Switch(true);
            MqttClient::instance()->publish("Schaltuhr/state", "bulb_on_4k0", true);
            break;
        }

        case bulb_on_2k7:
        {
            Switch(false);
            vTaskDelay(t_reset / portTICK_PERIOD_MS);
            Switch(true);
            MqttClient::instance()->publish("Schaltuhr/state", "bulb_on_2k7", true);
            break;
        }
        case bulb_unknown: // ignore
            break; 
        }
    }
}
