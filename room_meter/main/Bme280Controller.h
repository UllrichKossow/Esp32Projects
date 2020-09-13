#ifndef BME280CONTROLLER_H
#define BME280CONTROLLER_H

#include "esp_timer.h"

class Bme280Controller
{
public:
    Bme280Controller();
    void start();
    void timer_callback(void);


private:
    esp_timer_handle_t m_timer;
};

#endif // BME280CONTROLLER_H
