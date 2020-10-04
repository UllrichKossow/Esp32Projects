idf_component_register(SRCS app_main.cc Bme280Controller.cc set_time.cc connect.c time_helper.cc
                    INCLUDE_DIRS ".")

add_definitions(-DBME280_32BIT_ENABLE)
