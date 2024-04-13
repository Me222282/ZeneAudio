cmake_minimum_required(VERSION 3.5)
project(AudioBackend CXX)

# set(HEADER_FILES system.h)

# add_library(audioLib wasapiSystem.cpp ${HEADER_FILES})
# target_include_directories(audioLib PRIVATE ./)
# add_executable(main main.cpp ${HEADER_FILES})
# target_include_directories(main PRIVATE ./)

# windows test
<<<<<<< HEAD
# add_executable(main main.cpp wasapiSystem.cpp system.h)
=======
add_executable(main main.cpp wasapiSystem.cpp system.h readBuffer.h readBuffer.cpp)
>>>>>>> 8c14997a3a556ba4740dc9cc910d75b2c2e61fbd

# linux test
add_executable(main linuxTest.cpp)
#add_executable(main test.cpp)

find_package(ALSA REQUIRED)                                    
if (ALSA_FOUND)                                                
    target_include_directories(main PRIVATE ${ALSA_INCLUDE_DIRS})                  
    target_link_libraries (main ${ALSA_LIBRARIES})      
endif(ALSA_FOUND) 