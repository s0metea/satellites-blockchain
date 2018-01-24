/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SATELLITES_HELPER_H
#define SATELLITES_HELPER_H

#include <ns3/data-rate.h>
#include <ns3/satellite-channel.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>

namespace ns3 {
class SatellitesHelper {
public:
    NodeContainer ConfigureNodes(uint32_t nodes_amount, DataRate dataRate, Time time);
    const Ptr<SatelliteChannel> &getM_channel() const;
    std::vector<SatelliteChannel::Links> LoadLinks(std::string filename, int totalNodes);
private:
    Ptr<SatelliteChannel> m_channel;
    NetDeviceContainer m_netDevices;
public:
    const NetDeviceContainer &getM_netDevices() const;

    void setM_netDevices(const NetDeviceContainer &m_netDevices);
};

}

#endif /* SATELLITES_HELPER_H */

