#pragma once

class Bme280Adapter
{
private:
    static Bme280Adapter *m_instance;
    Bme280Adapter() = default;

public:
    Bme280Adapter(Bme280Adapter const &) = delete;
    void operator=(Bme280Adapter const &) = delete;

    static Bme280Adapter *instance();
};