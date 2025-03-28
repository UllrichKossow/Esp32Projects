idf_component_register(SRCS app_main.cc Bme280Controller.cc time_helper.cc
    INCLUDE_DIRS "."
	REQUIRES MqttClient SH1106 Bme280Adapter SetSystemTime json mqtt esp_pm)


