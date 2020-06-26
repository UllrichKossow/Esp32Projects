idf_component_register(SRCS "BME280_driver/bme280.c" 
  "SH1106_driver/sh1106.cc"
  "udp_logging/udp_logging.c"
  INCLUDE_DIRS "BME280_driver" "SH1106_driver" "udp_logging/include"
  )

add_definitions(-DBME280_32BIT_ENABLE)
