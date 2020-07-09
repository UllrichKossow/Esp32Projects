// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          

#include "protocol_examples_common.h"

#include "bme280_access.h"
#include "esp_log.h"

static const char *TAG = "BME280";


void app_main(void)
{    
    esp_log_level_set("*", ESP_LOG_INFO);
	
    ESP_ERROR_CHECK(example_connect());
    
	read_bme();
}
