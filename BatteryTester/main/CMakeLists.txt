idf_component_register(SRCS "MeasureController.cc" "AdcAccess.cc" "app_main.cc"
    INCLUDE_DIRS "."
	REQUIRES MqttClient SetSystemTime)
