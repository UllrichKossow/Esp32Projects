#include "Bme280Controller.h"
#include "time_helper.h"

#include "Bme280Adapter.h"
#include "MqttClient.h"
#include "esp_timer.h"

#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "cJSON.h"
#include "freertos/FreeRTOS.h"

// #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"

static const char *TAG = "Bme280Controller";

using namespace std;

static void to_timer_callback(void *arg) {
    Bme280Controller *ctrl = reinterpret_cast<Bme280Controller *>(arg);
    ctrl->timer_callback();
}

Bme280Controller::Bme280Controller() : m_cnt(0), m_maxSize(512) {}

void Bme280Controller::init() { Bme280Adapter::instance()->init(); }

void Bme280Controller::start() {
    esp_timer_init();
    esp_timer_create_args_t create_args = {.callback = to_timer_callback, .arg = this, .dispatch_method = ESP_TIMER_TASK, .name = "Bme280Controller"};
    m_currentInterval = 1 * 1000 * 1000;
    esp_timer_create(&create_args, &m_timer);
    esp_timer_start_periodic(m_timer, m_currentInterval);
}

void Bme280Controller::timer_callback() {
    ESP_LOGD(TAG, "timer_callback");
    ++m_cnt;

    checkCapacity();

    Bme280Adapter::data_t data;
    Bme280Adapter::instance()->read(data);
    addMeasure(data);
    publish(data);
    // ESP_LOGD(TAG, "%s", showData(data).c_str());
}

uint32_t Bme280Controller::getCounter() const { return m_cnt; }

uint32_t Bme280Controller::getCurrentInterval() const { return m_currentInterval; }

void Bme280Controller::checkCapacity() {
    m_measureLock.lock();
    if (m_measures.size() >= (m_maxSize - 1)) {
        if (m_currentInterval < 256 * 1000 * 1000) {
            m_currentInterval *= 2;
            esp_timer_stop(m_timer);
            esp_timer_start_periodic(m_timer, m_currentInterval);

            vector<measure_t> tmp;
            for (int i = 0; i < m_measures.size(); i += 2) {
                tmp.push_back(m_measures[i]);
            }
            m_measures = tmp;
        } else {
            vector<measure_t> tmp(m_measures.begin() + (m_maxSize / 4), m_measures.end());
            m_measures = tmp;
        }
    }
    m_measureLock.unlock();
}

string Bme280Controller::showData(const Bme280Adapter::data_t &data) {
    ostringstream s;

    s << "Temp " << fixed << setprecision(2) << 0.01 * data.temperature;
    s << "\tPabs " << fixed << setprecision(2) << 0.01 * data.pressure;
    s << "\tPcomp " << fixed << setprecision(2) << 0.01 * data.pressure / pow(1 - 570 / 44330.0, 5.255);
    s << "\tHum " << fixed << setprecision(3) << 0.001 * data.humidity;

    return s.str();
}

void Bme280Controller::addMeasure(const Bme280Adapter::data_t &data) {
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

std::string Bme280Controller::formatedNumber(double n, int precision) {
    ostringstream s;
    s << fixed << setprecision(precision) << n;
    return s.str();
}

void Bme280Controller::publish(const Bme280Adapter::data_t &data) {
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

timespec Bme280Controller::getDuration() {
    timespec duration = {0, 0};
    if (m_measures.size() > 1) {
        m_measureLock.lock();
        timespec t1 = m_measures.back().t;
        timespec t2 = m_measures.front().t;
        duration = timespec_sub(t1, t2);
        m_measureLock.unlock();
    }
    return duration;
}

uint32_t Bme280Controller::getNumberOfValues() { return m_measures.size(); }

vector<measure_t> Bme280Controller::getValuesForDuration(uint32_t count, uint32_t duration) {
    vector<measure_t> v(count);
    m_measureLock.lock();
    timespec t_last = m_measures.back().t;
    timespec t_first = t_last;
    t_first.tv_sec -= duration;

    int measureIdx = 0;

    for (int i = 0; i < count; ++i) {
        double offset = duration * double(i) / double(count);
        timespec t = t_first;
        t.tv_sec += (int)offset;
        while ((timespec_compare(m_measures[measureIdx].t, t) < 0) && (measureIdx < m_measures.size())) {
            ++measureIdx;
        }
        // ESP_LOGD(TAG, "offset=%f  v[%i] = m[%i]", offset, i, measureIdx);
        v[i] = m_measures[measureIdx];
    }

    m_measureLock.unlock();
    return v;
}
