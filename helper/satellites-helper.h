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
  NetDeviceContainer ConfigureNodes (uint32_t satellites, uint32_t groundStations, DataRate dataRate, Time time);
  const Ptr<SatelliteChannel> &getM_channel () const;
  std::vector<SatelliteChannel::Links> LoadLinks (std::string filename);
  const NodeContainer &getM_nodes() const;
  bool SetupLXC();
  bool CreateLXC();
  bool RunLXC();
  bool DestroyLXC();
  bool GenerateLXCConfigFiles();
  bool GenerateLXCRouteRules();
private:
  Ptr<SatelliteChannel> m_channel;
  NetDeviceContainer m_netDevices;
  NodeContainer m_nodes;
  NetDeviceContainer m_gatewayDevices, m_satelliteDevices, m_groundStationDevices;
  std::string ipToString(Ipv4InterfaceAddress address);
};

}

#endif /* SATELLITES_HELPER_H */

