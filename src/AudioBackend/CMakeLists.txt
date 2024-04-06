cmake_minimum_required(VERSION 3.5)
project(AudioBackend CXX)

# set(HEADER_FILES system.h)

# add_library(audioLib wasapiSystem.cpp ${HEADER_FILES})
# target_include_directories(audioLib PRIVATE ./)
# add_executable(main main.cpp ${HEADER_FILES})
# target_include_directories(main PRIVATE ./)

add_executable(main main.cpp wasapiSystem.cpp system.h)