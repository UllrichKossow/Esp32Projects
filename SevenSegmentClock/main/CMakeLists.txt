idf_component_register(SRCS app_main.cc 
                    INCLUDE_DIRS "."
                    REQUIRES esp_pm driver MqttClient SetSystemTime TM1637)
