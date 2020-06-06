idf_component_register(SRCS "BME280_driver/bme280.c"
                    INCLUDE_DIRS "BME280_driver"
                    REQUIRES console nvs_flash)
					
add_compile_definitions(${COMPONENT_LIB} "BME280_FLOAT_ENABLE")
