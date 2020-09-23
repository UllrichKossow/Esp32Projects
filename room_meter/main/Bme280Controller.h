#ifndef BME280CONTROLLER_H
#define BME280CONTROLLER_H

#include <string>

#include "esp_timer.h"

#include "bme280.h"

class Bme280Controller
{
public:
    Bme280Controller();
    void init();
    void start();
    void timer_callback(void);


private:
    esp_timer_handle_t m_timer;

    struct bme280_dev m_dev;
    uint32_t m_req_delay;

private:
    std::string showData(const bme280_data &data);
};

#endif // BME280CONTROLLER_H
