#include "Bme280Adapter.h"


Bme280Adapter *Bme280Adapter::instance()
{
    if (m_instance == nullptr)
    {
        m_instance = new Bme280Adapter();
    }
    return m_instance;
}