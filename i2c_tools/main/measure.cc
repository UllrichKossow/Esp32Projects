

#include <cstdint>

#include "bme280.h"

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	return 0;
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	return 0;
}

void user_delay_ms(uint32_t period)
{
	
}


void measure(void)
{
    struct bme280_dev dev;
    dev.dev_id = BME280_I2C_ADDR_PRIM;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_ms = user_delay_ms;
	
	bme280_init(&dev);
}