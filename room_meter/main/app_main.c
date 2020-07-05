// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          

#include "bme280_access.h"
#include "esp_log.h"

static const char *TAG = "BME280";


void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_WARN);
    read_bme();
}
