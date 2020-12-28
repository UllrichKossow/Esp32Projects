idf_component_register(SRCS "set_time.cc"
                    INCLUDE_DIRS "include"
					REQUIRES nvs_flash WlanConnect)
