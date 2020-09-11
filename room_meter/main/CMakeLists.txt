idf_component_register(SRCS app_main.cc Bme280Controller.cc bme280_access.cc set_time.cc Plot.cc connect.c DataCollector.cc time_helper.cc
                    INCLUDE_DIRS ".")

add_definitions(-DBME280_32BIT_ENABLE)
