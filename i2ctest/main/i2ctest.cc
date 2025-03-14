#include <cstdio>

const char *TAG = "i2ctest";

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/i2c_master.h"
#include "bme280.h"

static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    return 0;
}
static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    return 0;
}

static void user_delay_us(uint32_t usec, void *intf_ptr)
{
    auto t0 = esp_timer_get_time();
    auto t1 = t0 + usec;
    while(esp_timer_get_time() < t1)
    {

    }
}
void init_i2c()
{

    i2c_master_bus_handle_t bus_handle;
    if (!i2c_master_get_bus_handle(I2C_NUM_0, &bus_handle) == ESP_OK)
    {
        ESP_LOGI(TAG, "Init I2C");
        i2c_master_bus_config_t i2c_mst_config = {
            .i2c_port = 0,
            .clk_source = I2C_CLK_SRC_DEFAULT
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    }
}
void i2ctest()
{

    init_i2c();
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(I2C_NUM_0, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x76,
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
