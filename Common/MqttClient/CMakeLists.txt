idf_component_register(SRCS "MqttClient.cc"
                    INCLUDE_DIRS "."
                    REQUIRES mqtt 
					
                    PRIV_REQUIRES esp_netif)
