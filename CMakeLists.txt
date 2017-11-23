cmake_minimum_required(VERSION 3.8)
project(sat_blockchain)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        doc/sat-net-device.rst
        examples/sat-blockchain.cc
        examples/sat-net-device-example.cc
        examples/wscript
        helper/sat-net-device-helper.cc
        helper/sat-net-device-helper.h
        model/sat-channel.cc
        model/sat-channel.h
        model/sat-net-device.cc
        model/sat-net-device.h
        test/sat-net-device-test-suite.cc
        examples/json.hpp
        wscript)

include_directories(/media/sometea/Data/ns-3-dev/build/)
link_directories(/media/sometea/Data/ns-3-dev/build)

add_executable(sat_blockchain ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ns3-dev-*)