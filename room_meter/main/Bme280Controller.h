#ifndef BME280CONTROLLER_H
#define BME280CONTROLLER_H

#include <ctime>
#include <string>
#include <vector>
#include <mutex>

#include "esp_timer.h"

#include "bme280.h"

class Bme280Controller
{
public:
    Bme280Controller();
    void init();
    void start();
    void timer_callback(void);

    timespec getDuration();


public:
    struct measure_t
    {
        timespec t;
        double temp;
        double p_abs;
        double p_nn;
        double hum;
    };

private:
    esp_timer_handle_t m_timer;
    uint32_t m_currentInterval;
    uint32_t m_maxSize;

    struct bme280_dev m_dev;
    uint32_t m_req_delay;
    std::vector<measure_t> m_measures;
    std::mutex m_measureLock;

private:
    void addMeasure(const bme280_data &data);
    std::string showData(const bme280_data &data);
    void checkCapacity();
};

#endif // BME280CONTROLLER_H
