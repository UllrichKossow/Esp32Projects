// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-
#include "i2c_manager.h"

i2c_manager *i2c_manager::m_instance = nullptr;

i2c_manager::i2c_manager()
{
}

i2c_manager *i2c_manager::instance()
{
    if (!m_instance)
	m_instance = new i2c_manager();
    return m_instance;
}

void i2c_manager::begin_use()
{
    m_lock.lock();
}

void i2c_manager::end_use()
{
    m_lock.unlock();
}
