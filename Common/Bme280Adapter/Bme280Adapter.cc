#include "Bme280Adapter.h"
#include <cstring>

#include "bme280.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "Bme280Adapter";

static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    esp_err_t result = i2c_master_transmit_receive(*(Bme280Adapter::instance()->deviceHandle()), &reg_addr, 1, data, len, 1000);
    return 0;
}

static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    i2c_master_transmit_multi_buffer_info_t buffer[2];
    buffer[0].write_buffer = &reg_addr;
    buffer[0].buffer_size = 1;
    buffer[1].write_buffer = (uint8_t *)data;
    buffer[1].buffer_size = len;

    esp_err_t result =
        i2c_master_multi_buffer_transmit(*(Bme280Adapter::instance()->deviceHandle()), (i2c_master_transmit_multi_buffer_info_t *)&buffer, 2, 1000);

    return 0;
}

static void user_delay_us(uint32_t usec, void *intf_ptr) {
    auto t0 = esp_timer_get_time();
    auto t1 = t0 + usec;
    while (esp_timer_get_time() < t1) {
    }
}

Bme280Adapter *Bme280Adapter::m_instance = nullptr;

Bme280Adapter *Bme280Adapter::instance() {
    if (m_instance == nullptr) {
        m_instance = new Bme280Adapter();
    }
    if (!i2c_master_get_bus_handle(I2C_NUM_0, &m_instance->m_busHandle) == ESP_OK) {
        ESP_LOGE(TAG, "i2c_master_get_bus_handle failed.");
    }
    return m_instance;
}

void Bme280Adapter::init() {
    i2c_device_config_t dev_cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x76, .scl_speed_hz = 100000};

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_probe(m_busHandle, dev_cfg.device_address, 1000));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_probe(m_busHandle, dev_cfg.device_address, 1000));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(m_busHandle, &dev_cfg, &m_deviceHandle));

    m_bme280_dev.intf = BME280_I2C_INTF;
    m_bme280_dev.delay_us = user_delay_us;
    m_bme280_dev.read = user_i2c_read;
    m_bme280_dev.write = user_i2c_write;
    auto result = bme280_init(&m_bme280_dev);
    ESP_LOGI(TAG, "bme280_init() returned %i", result);

    /* Recommended mode of operation: Indoor navigation */

    bme280_settings settings;
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_t = BME280_OVERSAMPLING_1X;
    settings.filter = BME280_FILTER_COEFF_16;

    uint8_t settings_sel = BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP | BME280_SEL_OSR_HUM;

    /* Set the sensor settings */
    result = bme280_set_sensor_settings(settings_sel, &settings, &m_bme280_dev);
    uint32_t maxdelay;
    bme280_cal_meas_delay(&maxdelay, &settings);
}

void Bme280Adapter::release() { ESP_ERROR_CHECK(i2c_master_bus_rm_device(m_deviceHandle)); }

i2c_master_dev_handle_t *Bme280Adapter::deviceHandle() { return &m_deviceHandle; }

void Bme280Adapter::test() {

    bme280_settings settings;
    auto rslt = bme280_get_sensor_settings(&settings, &m_bme280_dev);

    settings.filter = BME280_FILTER_COEFF_2;

    /* Over-sampling rate for humidity, temperature and pressure */
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_t = BME280_OVERSAMPLING_1X;

    /* Setting the standby time */
    // settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

    rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &m_bme280_dev);

    uint32_t period;
    rslt = bme280_cal_meas_delay(&period, &settings);

    rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &m_bme280_dev);
    uint32_t info;
    m_bme280_dev.delay_us(period, &info);

    uint8_t status_reg;
    rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &m_bme280_dev);

    struct bme280_data comp_data;
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &m_bme280_dev);
    ESP_LOGI(TAG, "bme280_get_sensor_data() returned %i", rslt);
    ESP_LOGI(TAG, "temperature=%li", comp_data.temperature);
    ESP_LOGI(TAG, "humidity=%li", comp_data.humidity);
    ESP_LOGI(TAG, "pressure=%li", comp_data.pressure);
}