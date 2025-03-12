# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(COMPONENTS esptool_py esp32 driver freertos tcp_transport main)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(i2ctest)
