#ifndef BME280CONTROLLER_H
#define BME280CONTROLLER_H

#include <ctime>
#include <string>
#include <vector>
#include <mutex>
#include "Bme280Adapter.h"
#include "esp_timer.h"



struct measure_t
{
    timespec t;
    double temp;
    double p_abs;
    double p_nn;
    double hum;
};

class Bme280Controller
{
public:

public:
    Bme280Controller();
    void init();
    void start();
    void timer_callback(void);

    uint32_t getCounter() const ;
    uint32_t getCurrentInterval() const;

    timespec getDuration();
    uint32_t getNumberOfValues();

    std::vector<measure_t> getValuesForDuration(uint32_t count, uint32_t duration);
    std::vector<double> getTemperature();


private:
    uint32_t m_cnt;
    esp_timer_handle_t m_timer;
    uint32_t m_currentInterval;
    uint32_t m_maxSize;

    uint32_t m_req_delay;
    std::vector<measure_t> m_measures;
    std::mutex m_measureLock;

private:
    void addMeasure(const Bme280Adapter::data_t &data);
    std::string formatedNumber(double n, int precision);
    void publish(const Bme280Adapter::data_t &data);
    std::string showData(const Bme280Adapter::data_t &data);
    void checkCapacity();
};

#endif // BME280CONTROLLER_H
