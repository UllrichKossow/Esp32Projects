idf_component_register(SRCS "BME280_driver/bme280.c" "SH1106_driver/sh1106.c"
                    INCLUDE_DIRS "BME280_driver" "SH1106_driver"
                    REQUIRES console nvs_flash)
					
add_compile_definitions("BME280_FLOAT_ENABLE")
