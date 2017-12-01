cmake_minimum_required(VERSION 3.8)
project(satellites-blockchain)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        doc/satellites-blockchain.rst
        examples/json.hpp
        examples/satellites-blockchain.cc
        examples/satellites-net-device-example.cc
        helper/satellites-net-device-helper.cc
        helper/satellites-net-device-helper.h
        model/satellite-channel.cc
        model/satellite-channel.h
        model/satellite-net-device.cc
        model/satellite-net-device.h
        model/satellite.cpp
        model/satellite.h
        test/satellite-net-device-test-suite.cc
        wscript)

include_directories(/media/sometea/Data/ns-3-dev/build/)
link_directories(/media/sometea/Data/ns-3-dev/build)

add_executable(satellites-blockchain ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ns3-dev-*)
