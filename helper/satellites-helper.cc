#include <vector>
#include <string>
#include <ns3/node-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>
#include <ns3/core-module.h>
#include "satellites-helper.h"

namespace ns3 {
NodeContainer
SatellitesHelper::ConfigureNodes (uint32_t nodes_amount, DataRate dataRate, Time time)
{
  Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
  ObjectFactory m_propagationDelay;
  m_propagationDelay.SetTypeId ("ns3::ConstantSpeedPropagationDelayModel");
  Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
  channel->SetPropagationDelay (delay);
  m_channel = channel;
  NodeContainer nodes;
  nodes.Create (nodes_amount);
  for(uint32_t i = 0; i < nodes_amount; i++)
    {
      Ptr<SatelliteNetDevice> device = CreateObject<SatelliteNetDevice> ();
      device->SetAddress (Mac48Address::Allocate ());
      device->SetDataRate (dataRate);
      device->SetInterframeGap (time);
      device->SetChannel (channel);
      channel->Add (device);
      nodes.Get (i)->AddDevice (device);
      m_netDevices.Add (device);
    }
  return nodes;
}

const
Ptr<SatelliteChannel> &SatellitesHelper::getM_channel () const
{
  return m_channel;
}

const
NetDeviceContainer &SatellitesHelper::getM_netDevices () const
{
  return m_netDevices;
}

void
SatellitesHelper::setM_netDevices (const NetDeviceContainer &m_netDevices)
{
  SatellitesHelper::m_netDevices = m_netDevices;
}

std::vector<SatelliteChannel::Links>
SatellitesHelper::LoadLinks (std::string filename, int totalNodes){
  std::string time;
  std::vector<SatelliteChannel::Links> links;
  std::vector<std::vector<bool> > nodes;
  std::vector<bool> single_node;
  std::ifstream links_file;
  bool val;
  std::string row;
  links_file.open (filename.c_str (), std::ios_base::in);
  while(std::getline (links_file, time))
    {
      for(int i = 0; i < totalNodes; i++)
        {
          std::getline (links_file, row);
          std::istringstream iss (row);
          for (int j = 0; j < totalNodes; j++)
            {
              iss >> val;
              single_node.push_back (val);
            }
          nodes.push_back (single_node);
          single_node.clear ();
        }
      links.push_back (SatelliteChannel::Links (Time (time), nodes));
      nodes.clear ();
    }
  links_file.close ();
  return links;
}
}

