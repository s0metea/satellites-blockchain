#include <ns3/simulator.h>
#include <ns3/mobility-model.h>
#include <ns3/pointer.h>
#include <ns3/log.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/satellite-net-device.h>
#include "satellite-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ns3::SatelliteChannel");

NS_OBJECT_ENSURE_REGISTERED (SatelliteChannel);

TypeId
SatelliteChannel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatelliteChannel")
    .SetParent<Channel>()
    .SetGroupName ("Channel");
  return tid;
}

SatelliteChannel::SatelliteChannel()
{
}

SatelliteChannel::~SatelliteChannel()
{
  netDeviceList.clear ();
}

Ptr<NetDevice>
SatelliteChannel::GetDevice (uint32_t i) const {
  return netDeviceList[i];
}

void
SatelliteChannel::Add (Ptr<SatelliteNetDevice> device)
{
  netDeviceList.push_back (device);
}

uint32_t
SatelliteChannel::GetNDevices () const {
  return static_cast<uint32_t> (netDeviceList.size ());
}

void
SatelliteChannel::SetPropagationDelay (const Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}

void
SatelliteChannel::Send (Ptr<Packet> packet, uint16_t protocol, Address to, Ptr<SatelliteNetDevice> sender)
{
  NS_LOG_FUNCTION (this << packet << "From: " << sender->GetAddress () << " To: " << to);
  Ptr<MobilityModel> senderMobility = sender->GetNode ()->GetObject<MobilityModel>();
  NS_ASSERT (senderMobility != 0);
  NS_ASSERT (m_delay);
  NS_ASSERT (m_links.size ());
  std::vector<Ptr<NetDevice> > neighbours = sender->GetCommunicationNeighbors ();
  for (Ptr<NetDevice> device : neighbours)
    {
      Ptr<SatelliteNetDevice> neighbour = device->GetObject<SatelliteNetDevice>();
      NS_ASSERT (device->GetNode () != 0);
      NS_ASSERT (sender != device);
      if(neighbour->GetAddress () == Mac48Address::ConvertFrom (to) || Mac48Address::ConvertFrom (to) == device->GetBroadcast ())
        {
          Ptr<MobilityModel> receiverMobility = device->GetNode ()->GetObject<MobilityModel>();
          NS_ASSERT (receiverMobility != 0);
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
          NS_LOG_DEBUG ("Propagation Delay Node" << sender->GetNode ()->GetId ()
                                                 << " --> Node" << device->GetNode ()->GetId ()
                                                 << " = " << delay);
          NS_LOG_DEBUG ("The distance = " << senderMobility->GetDistanceFrom (receiverMobility));
          Simulator::ScheduleWithContext (neighbour->GetNode ()->GetId (), delay, &SatelliteNetDevice::StartRX,
                                          neighbour,
                                          packet->Copy (),
                                          sender->GetAddress (),
                                          protocol);
        }
    }
}

SatelliteChannel::Links::Links(Time time, std::vector<std::vector<bool> > links)
{
  m_time = time;
  m_links = links;
}

std::vector<SatelliteChannel::Links>
SatelliteChannel::GetLinks ()
{
  return m_links;
}

void
SatelliteChannel::SetLinks (std::vector<SatelliteChannel::Links> links)
{
  m_links = links;
}
}
