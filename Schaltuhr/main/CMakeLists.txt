idf_component_register(SRCS app_main.cc TimeSwitch.cc RfSwitch.cc
    INCLUDE_DIRS "."
	REQUIRES driver esp_hw_support MqttClient SetSystemTime json tcp_transport)
