#include <cstdio>

const char *TAG="i2ctest";

#include "esp_log.h"
#include "driver/i2c_master.h"


void i2ctest() {

}

extern "C" void app_main(void);
void app_main(void)
{
    ESP_LOGI(TAG, "Start...");
    i2ctest();
    while (1)
    {
        
    }
}
