cmake_minimum_required(VERSION 3.8)
project(satellites-blockchain)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DNS3_LOG_ENABLE)

include_directories(/home/mike/ns-3-dev/build/)
link_directories(/home/mike/ns-3-dev/build)

set(SOURCE_FILES
        examples/satellites-blockchain-example.cc
        model/satellite-net-device.cc
        model/satellite-channel.cc
        model/satellite-net-device.h
        model/satellite-channel.h
        helper/satellites-helper.cc
        helper/satellites-helper.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        ns3-dev-core-debug
        ns3-dev-network-debug
        ns3-dev-internet-debug
        ns3-dev-mobility-debug
        ns3-dev-flow-monitor-debug
        ns3-dev-satellites-blockchain-debug
        ns3-dev-internet-apps-debug
        ns3-dev-applications-debug
        ns3-dev-propagation-debug
        ns3-dev-low-resolution-radio-debug
        ns3-dev-tap-bridge-debug)