idf_component_register(SRCS "i2ctools_example_main.c"
                            "cmd_i2ctools.c" "measure.cc"
                    INCLUDE_DIRS ".")

add_compile_definitions("BME280_FLOAT_ENABLE")
