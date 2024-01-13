idf_component_register(SRCS app_main.cc Bme280Controller.cc time_helper.cc
    INCLUDE_DIRS "."
	REQUIRES esp_pm MqttClient I2cManager SH1106 BME280 SetSystemTime json)

add_definitions(-DBME280_32BIT_ENABLE)
