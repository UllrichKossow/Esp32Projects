#include "MeasureController.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "MeasureController";

using namespace std;

static void to_timer_callback(void *arg)
{
    MeasureController *ctrl = reinterpret_cast<MeasureController*>(arg);
    ctrl->timer_callback();
}

void MeasureController::Start()
{
    esp_timer_init();
    esp_timer_create_args_t create_args = {
        .callback = to_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Bme280Controller"
    };
    m_currentInterval = 1*1000*1000;
    esp_timer_create(&create_args, &m_timer);
    esp_timer_start_periodic(m_timer, m_currentInterval);
}

void MeasureController::timer_callback()
{
    ESP_LOGD(TAG, "timer_callback");
}