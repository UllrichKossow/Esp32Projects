idf_component_register(SRCS app_main.cc TimeSwitch.cc RfSwitch.cc
    INCLUDE_DIRS "."
	REQUIRES MqttClient SetSystemTime json tcp_transport)
