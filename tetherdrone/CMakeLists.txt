cmake_minimum_required(VERSION 3.16.0)

# Add this line to tell ESP-IDF to look in the 'lib' folder for components
set(EXTRA_COMPONENT_DIRS "lib")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tetherdrone)