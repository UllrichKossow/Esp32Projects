#ifndef BME280CONTROLLER_H
#define BME280CONTROLLER_H

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
};

#endif // BME280CONTROLLER_H
