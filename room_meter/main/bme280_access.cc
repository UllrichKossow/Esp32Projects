// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include <cstdint>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "driver/i2c.h"
#include "bme280.h"
#include "sh1106.h"
#include "i2c_manager.h"

using namespace std;

//-----------------------------------------------------------------------------------------------------

static const char *TAG = "measure";


int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_manager::instance()->GetCmdHandle();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

    if (len > 1) {
        i2c_master_read(cmd, data, len-1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data+len-1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_manager::instance()->ReleaseCmdHandle(cmd);

    return (espRc == ESP_OK) ? 0 : -1;
}

int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
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

void user_delay_ms(uint32_t msek)
{
    vTaskDelay(msek/portTICK_PERIOD_MS);
}


//-----------------------------------------------------------------------------------------------------
std::string show_data_string(const bme280_data *comp_data)
{
    ostringstream s;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    struct tm timeinfo;
    setenv("TZ", "UTC-2", 1);
    tzset();
    localtime_r(&now.tv_sec, &timeinfo);
    char str_time[64];
    char str_date[64];
    strftime(str_time, sizeof(str_time), "%T", &timeinfo);
    strftime(str_date, sizeof(str_time), "%F", &timeinfo);

    s << "T " << fixed << setprecision(2) << 0.01 * comp_data->temperature;
    sh1106_print_line(0, s.str().c_str());
    s.str("");
    s << "P " << fixed << setprecision(2) << 0.01 * comp_data->pressure;
    sh1106_print_line(1, s.str().c_str());
    s.str("");
    s << "P " << fixed << setprecision(2) << 0.01 * comp_data->pressure / pow(1 - 570/44330.0, 5.255);
    sh1106_print_line(2, s.str().c_str());
    s.str("");
    s << "H " << fixed << setprecision(3) << 0.001 * comp_data->humidity;
    sh1106_print_line(3, s.str().c_str());

    return s.str();
}

void show_date_time()
{
    ostringstream s;
    struct timespec now_rt, now_mo;
    clock_gettime(CLOCK_REALTIME, &now_rt);
    clock_gettime(CLOCK_MONOTONIC, &now_mo);
    //ESP_LOGI(TAG, "t2=%li %li", now_rt.tv_sec, now_rt.tv_nsec);
    struct tm timeinfo;
    setenv("TZ", "UTC-2", 1);
    tzset();
    localtime_r(&now_rt.tv_sec, &timeinfo);
    char str_time[64];
    char str_date[64];
    strftime(str_time, sizeof(str_time), "%T", &timeinfo);
    strftime(str_date, sizeof(str_time), "%F", &timeinfo);
    s << str_time << " " << now_rt.tv_nsec/1000000 ;
    
    sh1106_print_line(4,str_date);
    sh1106_print_line(5,s.str().c_str());
    
    s.str("");
    s << now_mo.tv_sec << " " << now_mo.tv_nsec/1000000 << " " << now_mo.tv_sec % 3600;
    sh1106_print_line(6, s.str().c_str());    
}

//-----------------------------------------------------------------------------------------------------
/*!
 * @brief This API reads the sensor temperature, pressure and humidity data in forced mode.
 */
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev)
{
    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    /* Variable to define the selecting sensors */
    uint8_t settings_sel = 0;

    /* Variable to store minimum wait time between consecutive measurement in force mode */
    uint32_t req_delay;

    /* Structure to get the pressure, temperature and humidity values */
    struct bme280_data comp_data;

    /* Recommended mode of operation: Indoor navigation */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    /* Set the sensor settings */
    rslt = bme280_set_sensor_settings(settings_sel, dev);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", rslt);

        return rslt;
    }


    /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
     *  and the oversampling configuration. */
    req_delay = bme280_cal_meas_delay(&dev->settings);

    /* Continuously stream sensor data */
    int n = 0;
    while (1)
    {
        struct timespec now;

        clock_gettime(CLOCK_REALTIME, &now);

        if ((now.tv_sec % 10 == 0) || (n++ < 10))
        {
            /* Set the sensor to forced mode */
            rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
            if (rslt != BME280_OK)
            {
                fprintf(stderr, "Failed to set sensor mode (code %+d).", rslt);
                break;
            }

            /* Wait for the measurement to complete and print data */
            dev->delay_ms(req_delay);
            rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
            if (rslt != BME280_OK)
            {
                fprintf(stderr, "Failed to get sensor data (code %+d).", rslt);
                break;
            }

            //print_sensor_data(&comp_data);
            show_data_string(&comp_data);
        }

        struct timespec now_rt;
        clock_gettime(CLOCK_REALTIME, &now_rt);
        ESP_LOGI(TAG, "t1=%li %li", now_rt.tv_sec, now_rt.tv_nsec);
#if 1
        int ms = now_rt.tv_nsec/1000000;
        int delay_ms = 1050 - ms;
        if (delay_ms < 1)
            delay_ms = 1;
        vTaskDelay(delay_ms/portTICK_PERIOD_MS);
#else
        int us = now_rt.tv_nsec/1000;
        int delay_us = 1050000 - us;
        if (delay_us < 1)
            delay_us = 1;
        esp_sleep_enable_timer_wakeup(delay_us);
        esp_wifi_stop();
        esp_light_sleep_start();
        esp_wifi_start();
#endif    
        show_date_time();
    }

    return rslt;
}

extern "C" void read_bme(void);
void sync_time(void);


void read_bme()
{
    struct bme280_dev dev;

    /* Variable to define the result */
    int8_t rslt = BME280_OK;


    /* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
    dev.dev_id = BME280_I2C_ADDR_PRIM;

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_ms = user_delay_ms;


    sh1106_init();
    task_sh1106_display_clear(NULL);
    sh1106_print_line(0, "Sync time...");
    
    sync_time();
    task_sh1106_display_clear(NULL);

    /* Initialize the bme280 */
    rslt = bme280_init(&dev);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }

    rslt = stream_sensor_data_forced_mode(&dev);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to stream sensor data (code %+d).\n", rslt);
        exit(1);
    }
}
