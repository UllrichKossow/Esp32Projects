idf_component_register(SRCS app_main.cc 
                    INCLUDE_DIRS "."
                    REQUIRES MqttClient SetSystemTime TM1637)
