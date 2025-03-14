idf_component_register(SRCS "Bme280Adapter.cc" "i2ctest.cc"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_i2c esp_timer newBME280)
