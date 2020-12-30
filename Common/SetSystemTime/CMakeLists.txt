idf_component_register(SRCS SetSystemTime.cc
                    INCLUDE_DIRS "."
					REQUIRES nvs_flash WlanConnect)
