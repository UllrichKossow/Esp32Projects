#include "Bme280Controller.h"

#include "i2c_manager.h"
#include "driver/i2c.h"

#include <ctime>

#include "esp_log.h"

static const char* TAG = "Bme280Controller";

static void to_timer_callback(void *arg)
{
    Bme280Controller *ctrl = reinterpret_cast<Bme280Controller*>(arg);
    ctrl->timer_callback();
}

static int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_manager::instance()->GetCmdHandle();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

    if (len > 1)
    {
        i2c_master_read(cmd, data, len-1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data+len-1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_manager::instance()->ReleaseCmdHandle(cmd);

    return (espRc == ESP_OK) ? 0 : -1;
}

static int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    esp_err_t espRc;
    i2c_cmd_handle_t cmd = i2c_manager::instance()->GetCmdHandle();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);

    i2c_manager::instance()->ReleaseCmdHandle(cmd);

    return (espRc == ESP_OK) ? 0 : -1;
}

static void user_delay_ms(uint32_t msek)
{
    vTaskDelay(msek/portTICK_PERIOD_MS);
}




Bme280Controller::Bme280Controller()
{

}

void Bme280Controller::init()
{
    m_dev.dev_id = BME280_I2C_ADDR_PRIM;

    m_dev.intf = BME280_I2C_INTF;
    m_dev.read = user_i2c_read;
    m_dev.write = user_i2c_write;
    m_dev.delay_ms = user_delay_ms;
    int8_t rslt = bme280_init(&m_dev);

    /* Recommended mode of operation: Indoor navigation */
    m_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    m_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    m_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    m_dev.settings.filter = BME280_FILTER_COEFF_16;

    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    /* Set the sensor settings */
    rslt = bme280_set_sensor_settings(settings_sel, &m_dev);
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