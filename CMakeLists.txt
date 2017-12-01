cmake_minimum_required(VERSION 3.8)
project(satellites-blockchain)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        examples/satellites-net-device-example.cc
        model/satellite-channel.h
        model/satellite.h
        model/satellite-net-device.h
        model/satellite-channel.cc
        model/satellite-net-device.cc
        model/satellite.cpp
        wscript)

include_directories(~/NS3/ns-3-dev/build/)
link_directories(~NS3/ns-3-dev/build)

add_executable(satellites-blockchain ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ns3-dev-*)
