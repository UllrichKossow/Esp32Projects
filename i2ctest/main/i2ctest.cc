
#include "freertos/FreeRTOS.h"
const char *TAG = "i2ctest";

#include "driver/i2c_master.h"
#include "esp_log.h"

#include "Bme280Adapter.h"
#include "sh1106.h"

void init_i2c() {

    i2c_master_bus_handle_t bus_handle;
    if (!i2c_master_get_bus_handle(I2C_NUM_0, &bus_handle) == ESP_OK) {
        ESP_LOGI(TAG, "Init I2C");
        i2c_master_bus_config_t i2c_mst_config = {.i2c_port = 0,
                                                  .sda_io_num = GPIO_NUM_21,
                                                  .scl_io_num = GPIO_NUM_22,
                                                  .clk_source = I2C_CLK_SRC_DEFAULT,
                                                  .glitch_ignore_cnt = 7,
                                                  .flags = {.enable_internal_pullup = 1}};
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    }
}

void play_display() {
    SH1106::instance()->init();
    for (int i = 0; i < 8; ++i) {

        SH1106::instance()->display_clear();
        SH1106::instance()->print_line(i, "Hallo World!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    SH1106::instance()->display_clear();
}

extern "C" void app_main(void);

void app_main(void) {
    ESP_LOGI(TAG, "Start...");
    init_i2c();
    uint32_t cnt = 0;
    play_display();
    while (1) {
        Bme280Adapter::instance()->init();
        Bme280Adapter::instance()->test();
        Bme280Adapter::instance()->release();
        ESP_LOGI(TAG, "Loop (%lu)...", cnt++);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
