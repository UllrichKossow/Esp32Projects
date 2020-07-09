// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-
#include "i2c_manager.h"

#include "esp_log.h"

//-----------------------------------------------------------------------------------------------------
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

static const char *TAG = "i2c_manager";

static uint32_t i2c_frequency = 100000;
static i2c_port_t i2c_port = I2C_NUM_0;


i2c_manager *i2c_manager::m_instance = nullptr;

i2c_manager::i2c_manager()
    :m_isDriverInstalled(false)
{
}

i2c_manager *i2c_manager::instance()
{
    if (!m_instance)
	m_instance = new i2c_manager();
    return m_instance;
}

i2c_cmd_handle_t i2c_manager::GetCmdHandle()
{
    m_lock.lock();
    if (!m_isDriverInstalled)
    {
        i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
        m_isDriverInstalled = true;
    }

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.scl_io_num = 22;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = i2c_frequency;

    i2c_param_config(0, &conf);

    m_cmd = i2c_cmd_link_create();
    return m_cmd; 
        
}

void i2c_manager::ReleaseCmdHandle(const i2c_cmd_handle_t &cmd) 
{
    i2c_cmd_link_delete(m_cmd);

    m_lock.unlock();
}

void i2c_manager::CloseDriver()
{
    i2c_driver_delete(i2c_port);
}
