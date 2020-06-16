idf_component_register(SRCS "i2ctools_example_main.c" "measure.cc" "set_time.cc"
                    INCLUDE_DIRS ".")

add_definitions(-DBME280_32BIT_ENABLE)
