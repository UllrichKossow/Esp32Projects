idf_component_register(
  SRCS  "Bme280Adapter.cc" "bme280.c"
  INCLUDE_DIRS "include" .
  REQUIRES esp_driver_i2c esp_timer)

add_definitions(-DBME280_32BIT_ENABLE)


#https://github.com/BoschSensortec/BME280_driver.git
