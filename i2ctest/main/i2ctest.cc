#include <cstdio>

const char *TAG="i2ctest";

#include "esp_log.h"
#include "driver/i2c_master.h"



void i2ctest() {
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,

    };
    
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x58,
        .scl_speed_hz = 100000,
    };
    
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

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
