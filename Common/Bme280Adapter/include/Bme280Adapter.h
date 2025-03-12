#pragma once
#include "bme280.h"
#include "driver/i2c_master.h"

class Bme280Adapter {
  private:
    static Bme280Adapter *m_instance;
    Bme280Adapter() = default;
    i2c_master_bus_handle_t m_busHandle;
    i2c_master_dev_handle_t m_deviceHandle;
    bme280_dev m_bme280_dev;

  public:
    static Bme280Adapter *instance();

    Bme280Adapter(Bme280Adapter const &) = delete;
    void operator=(Bme280Adapter const &) = delete;

    void init();
    void release();
    i2c_master_dev_handle_t *deviceHandle();

    void test();
};