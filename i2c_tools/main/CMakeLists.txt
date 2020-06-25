idf_component_register(SRCS app_main.c bme280_access.cc "set_time.cc"
                    INCLUDE_DIRS ".")

add_definitions(-DBME280_32BIT_ENABLE)
