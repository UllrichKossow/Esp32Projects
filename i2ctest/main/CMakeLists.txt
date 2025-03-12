idf_component_register(SRCS "i2ctest.cc"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_i2c esp_timer Bme280Adapter)
                    
add_definitions(-DBME280_32BIT_ENABLE)
