#include "Bme280Controller.h"
#include "time_helper.h"

#include "i2c_manager.h"
#include "driver/i2c.h"
#include "MqttClient.h"

#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "freertos/FreeRTOS.h"
#include "cJSON.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"

static const char* TAG = "Bme280Controller";

using namespace std;

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
    : m_cnt(0), m_maxSize(512)
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
    m_req_delay = bme280_cal_meas_delay(&m_dev.settings);
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
    m_currentInterval = 1*1000*1000;
    esp_timer_create(&create_args, &m_timer);
    esp_timer_start_periodic(m_timer, m_currentInterval);
}


void Bme280Controller::timer_callback()
{
    ESP_LOGD(TAG, "timer_callback");
    ++m_cnt;

    checkCapacity();

    int8_t rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &m_dev);
    if (rslt != BME280_OK)
    {
        ESP_LOGE(TAG, "bme280_set_sensor_mode() failed.");
        return;
    }
    m_dev.delay_ms(m_req_delay);
    bme280_data comp_data;
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &m_dev);
    if (rslt != BME280_OK)
    {
        ESP_LOGE(TAG, "bme280_get_sensor_data() failed.");
        return;
    }
    addMeasure(comp_data);
    publish(comp_data);
    //ESP_LOGD(TAG, "%s", showData(comp_data).c_str());
}

uint32_t Bme280Controller::getCounter() const
{
    return m_cnt;
}

uint32_t Bme280Controller::getCurrentInterval() const
{
    return m_currentInterval;
}

void Bme280Controller::checkCapacity()
{
    m_measureLock.lock();
    if (m_measures.size() >= (m_maxSize - 1))
    {
        if (m_currentInterval < 256*1000*1000)
        {
            m_currentInterval *= 2;
            esp_timer_stop(m_timer);
            esp_timer_start_periodic(m_timer, m_currentInterval);

            vector<measure_t> tmp;
            for (int i = 0; i < m_measures.size(); i += 2)
            {
                tmp.push_back(m_measures[i]);
            }
            m_measures = tmp;
        }
        else
        {
            vector<measure_t> tmp(m_measures.begin()+(m_maxSize/4), m_measures.end());
            m_measures = tmp;
        }
    }
    m_measureLock.unlock();
}


string Bme280Controller::showData(const bme280_data &data)
{
    ostringstream s;

    s << "Temp " << fixed << setprecision(2) << 0.01 * data.temperature;
    s << "\tPabs " << fixed << setprecision(2) << 0.01 * data.pressure;
    s << "\tPcomp " << fixed << setprecision(2) << 0.01 * data.pressure / pow(1 - 570/44330.0, 5.255);
    s << "\tHum " << fixed << setprecision(3) << 0.001 * data.humidity;

    return s.str();
}


void Bme280Controller::addMeasure(const bme280_data &data)
{
    ESP_LOGD(TAG, "%s heap=%i, values=%i, interval=%lu", __FUNCTION__, xPortGetFreeHeapSize(), m_measures.size(), m_currentInterval);

    measure_t m;
    clock_gettime(CLOCK_MONOTONIC, &m.t);
    m.temp = 0.01 * data.temperature;
    m.hum = 0.001 * data.humidity;
    m.p_abs = 0.01 * data.pressure;
    m.p_nn = 0.01 * data.pressure / pow(1 - 570 / 44330.0, 5.255);
    m_measureLock.lock();
    m_measures.push_back(m);
    m_measureLock.unlock();
}

std::string Bme280Controller::formatedNumber(double n, int precision)
{
    ostringstream s;
    s << fixed << setprecision(precision) << n;
    return s.str();
}

void Bme280Controller::publish(const bme280_data &data)
{
    cJSON *measure = cJSON_CreateObject();
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    cJSON *d = cJSON_AddObjectToObject(measure, "sensordata");
    cJSON_AddRawToObject(d, "time", formatedNumber(now.tv_sec, 0).c_str());
    cJSON_AddRawToObject(d, "temperature", formatedNumber(0.01 * data.temperature, 2).c_str());
    cJSON_AddRawToObject(d, "humidity", formatedNumber(0.001 * data.humidity, 2).c_str());
    cJSON_AddRawToObject(d, "pressure", formatedNumber(0.01 * data.pressure, 2).c_str());
    cJSON_AddRawToObject(d, "pressure_nn", formatedNumber(0.01 * data.pressure / pow(1 - 570 / 44330.0, 5.255), 2).c_str());
    char *text = cJSON_Print(measure);
    MqttClient::instance()->publish("EspMeasure", text);
    ESP_LOGD(TAG, "JSON=%s", text);
    free(text);
    cJSON_Delete(measure);
}

    timespec Bme280Controller::getDuration()
{
    timespec duration = {0, 0};
    if (m_measures.size() > 1)
    {
        m_measureLock.lock();
        timespec t1 = m_measures.back().t;
        timespec t2 = m_measures.front().t;
        duration = timespec_sub(t1, t2);
        m_measureLock.unlock();
    }
    return duration;
}


uint32_t Bme280Controller::getNumberOfValues()
{
    return m_measures.size();
}

vector<measure_t> Bme280Controller::getValuesForDuration(uint32_t count, uint32_t duration)
{
    vector<measure_t> v(count);
    m_measureLock.lock();
    timespec t_last = m_measures.back().t;
    timespec t_first = t_last;
    t_first.tv_sec -= duration;

    int measureIdx = 0;

    for (int i = 0; i < count; ++i)
    {
        double offset = duration * double(i) / double(count);
        timespec t = t_first;
        t.tv_sec += (int) offset;
        while ((timespec_compare(m_measures[measureIdx].t, t) < 0) && (measureIdx < m_measures.size()))
        {
            ++measureIdx;
        }
        //ESP_LOGD(TAG, "offset=%f  v[%i] = m[%i]", offset, i, measureIdx);
        v[i] = m_measures[measureIdx];
    }

    m_measureLock.unlock();
    return v;
}
