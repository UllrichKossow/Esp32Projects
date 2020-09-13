#include "Bme280Controller.h"


#include <ctime>

#include "esp_log.h"

static const char* TAG = "Bme280Controller";

void to_timer_callback(void *arg)
{
    Bme280Controller *ctrl = reinterpret_cast<Bme280Controller*>(arg);
    ctrl->timer_callback();
}

Bme280Controller::Bme280Controller()
{

}


void Bme280Controller::start()
{
    esp_timer_init();
    esp_timer_create_args_t create_args = {
        .callback = to_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Bme280Controller"
    };

    esp_timer_create(&create_args, &m_timer);
    esp_timer_start_periodic(m_timer, 1000000);
}

void Bme280Controller::timer_callback()
{
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    int64_t now = esp_timer_get_time();
    ESP_LOGD(TAG, "timer %li %li", t.tv_sec, t.tv_nsec);
}
