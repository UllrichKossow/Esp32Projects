idf_component_register(SRCS app_main.cc TimeSwitch.cc RfSwitch.cc MqttClient.cc
    INCLUDE_DIRS "."
	REQUIRES mqtt SetSystemTime)
