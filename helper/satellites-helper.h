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
  NetDeviceContainer ConfigureNodes (uint32_t nodes_amount, DataRate dataRate, Time time);
  const Ptr<SatelliteChannel> &getM_channel () const;
  std::vector<SatelliteChannel::Links> LoadLinks (std::string filename, int totalNodes);
  const NodeContainer &getM_nodes() const;
  bool SetupLXC();
  bool CreateLXC();
  bool RunLXC();
  bool DestroyLXC();
  bool GenerateLXCConfigFiles();
private:
  Ptr<SatelliteChannel> m_channel;
  NetDeviceContainer m_netDevices;
  NodeContainer m_nodes;
  std::string ipToString(Ipv4InterfaceAddress address);
};

}

#endif /* SATELLITES_HELPER_H */

