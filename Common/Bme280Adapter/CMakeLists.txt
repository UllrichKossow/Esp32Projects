idf_component_register(
  SRCS  "Bme280Adapter.cc" "bme280.c"
  INCLUDE_DIRS .

  REQUIRES esp_driver_i2c 
  PRIV_REQUIRES esp_timer)

add_definitions(-DBME280_32BIT_ENABLE)


#https://github.com/BoschSensortec/BME280_driver.git
