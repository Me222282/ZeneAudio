cmake_minimum_required(VERSION 3.5)
project(AudioBackend CXX)

# set(HEADER_FILES system.h)

# add_library(audioLib wasapiSystem.cpp ${HEADER_FILES})
# target_include_directories(audioLib PRIVATE ./)
# add_executable(main main.cpp ${HEADER_FILES})
# target_include_directories(main PRIVATE ./)

# windows test
if (WIN32)
    # add_executable(main main.cpp wasapiSystem.cpp system.h readBuffer.h readBuffer.cpp)
    add_library(audio_wasapi SHARED wasapiSystem.cpp system.h readBuffer.h readBuffer.cpp)
    set_target_properties(audio_wasapi PROPERTIES PUBLIC_HEADER system.h)
    target_compile_definitions(audio_wasapi PRIVATE WINDOWS)
endif()

# linux test
if (UNIX)
    add_executable(main linuxTest.cpp system.h alsaSystem.cpp)
    target_compile_definitions(main PRIVATE UNIX)
    # add_executable(main test.cpp)

    find_package(ALSA REQUIRED)                                    
    if (ALSA_FOUND)                                                
        target_include_directories(main PRIVATE ${ALSA_INCLUDE_DIRS})                  
        target_link_libraries (main ${ALSA_LIBRARIES})      
    endif(ALSA_FOUND) 
endif()