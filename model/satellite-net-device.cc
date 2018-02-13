#include "satellite-net-device.h"
#include <ns3/network-module.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ns3::SatelliteNetDevice");

NS_OBJECT_ENSURE_REGISTERED (SatelliteNetDevice);

TypeId
SatelliteNetDevice::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatelliteNetDevice")
    .SetParent<lrr::NeighborAwareDevice>()
    .SetGroupName ("NetDevice")
    .AddAttribute ("DataRate",
                   "The data rate that the Net Device uses to simulate packet transmission timing.",
                   DataRateValue (DataRate ("1250MB/s")),
                   MakeDataRateAccessor (&SatelliteNetDevice::bps),
                   MakeDataRateChecker ())
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (DEFAULT_MTU),
                   MakeUintegerAccessor (&SatelliteNetDevice::SetMtu,
                                         &SatelliteNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t>())
    .AddAttribute ("InterframeGap",
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (MicroSeconds (static_cast<uint64_t> (10.0))),
                   MakeTimeAccessor (&SatelliteNetDevice::m_InterframeGap),
                   MakeTimeChecker ());
  return tid;
}

SatelliteNetDevice::SatelliteNetDevice()
  : m_txMachineState (READY),
    m_channel (nullptr),
    m_linkUp (false)
{
  NS_LOG_FUNCTION (this);
  m_queue = CreateObject<DropTailQueue<Packet> >();
}

SatelliteNetDevice::~SatelliteNetDevice()
{
  NS_LOG_FUNCTION (this);
}

void
SatelliteNetDevice::SetIfIndex (const uint32_t index)
{

}

void
SatelliteNetDevice::SetDataRate (DataRate m_bps)
{
  m_bps = bps;
}

Ptr<Channel>
SatelliteNetDevice::GetChannel (void) const
{
  return m_channel;
}

void
SatelliteNetDevice::SetChannel (Ptr<SatelliteChannel> channel)
{
  m_channel = channel;
}

void
SatelliteNetDevice::SetAddress (Address address)
{
  m_address = Mac48Address::ConvertFrom (address);
}

Address
SatelliteNetDevice::GetAddress (void) const
{
  return m_address;
}

uint16_t
SatelliteNetDevice::GetMtu (void) const
{
  return static_cast<uint16_t> (m_mtu);
}


bool
SatelliteNetDevice::SupportsSendFrom (void) const
{
  return true;
}

void
SatelliteNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (&cb);
  m_promiscRxCallback = cb;
}

Address
SatelliteNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address
SatelliteNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

void
SatelliteNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

bool
SatelliteNetDevice::NeedsArp (void) const
{
  return true;
}

void
SatelliteNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
}

Ptr<Node>
SatelliteNetDevice::GetNode (void) const
{
  return m_node;
}


bool
SatelliteNetDevice::Send (Ptr<Packet> packet, const Address &dest, uint16_t protocol)
{
  NS_LOG_FUNCTION (this << packet << dest << protocol);
  SendFrom (packet, m_address, dest, protocol);
  return true;
}

bool SatelliteNetDevice::SendFrom (Ptr<Packet> packet, const Address &source, const Address &dest,
                                   uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);

  //EthernetHeader:
  EthernetHeader ethernetHeader (false);
  ethernetHeader.SetSource (Mac48Address::ConvertFrom (source));
  ethernetHeader.SetDestination (Mac48Address::ConvertFrom (dest));

  //LLCHeader:
  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

  packet->AddHeader (ethernetHeader);

  m_queue->Enqueue (packet);

  if (m_txMachineState == READY)
    {
      TX ();
    }
  return true;
}

bool
SatelliteNetDevice::TX ()
{
  NS_LOG_FUNCTION (this);
  if (m_queue->GetNPackets () == 0)
    {
      m_txMachineState = READY;
      return true;
    }
  m_txMachineState = BUSY;
  NS_ASSERT (!m_txEvent.IsRunning ());
  Ptr<Packet> m_currentPkt = m_queue->Dequeue ();
  m_currentTxPacket = m_currentPkt;
  EthernetHeader ethernetHeader;
  m_currentPkt->PeekHeader (ethernetHeader);
  Time totalTime = m_InterframeGap + bps.CalculateBytesTxTime (m_currentPkt->GetSize ());
  m_channel->Send (m_currentPkt, this);
  m_txEvent = Simulator::Schedule (totalTime, &SatelliteNetDevice::TX, this);
  return true;
}

DataRate
SatelliteNetDevice::GetDataRate ()
{
  return bps;
}

bool
SatelliteNetDevice::StartRX (Ptr<Packet> packet)
{
  Time totalTime = m_InterframeGap + bps.CalculateBytesTxTime (packet->GetSize ());
  NS_LOG_FUNCTION ("RX duration: " << totalTime);
  Simulator::Schedule (totalTime, &SatelliteNetDevice::ForwardUp, this, packet);
  return true;
}


bool SatelliteNetDevice::ForwardUp (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("Device: " << m_address << " " << packet->GetUid ());
  EthernetHeader eh (false);
  LlcSnapHeader llc;
  packet->RemoveHeader (eh);
  packet->RemoveHeader (llc);
  Mac48Address from = eh.GetSource ();
  Mac48Address dst = eh.GetDestination ();
  NetDevice::PacketType type;
  if (dst.IsBroadcast ())
    {
      type = NetDevice::PACKET_BROADCAST;
    } else if (dst.IsGroup ())
    {
      type = NetDevice::PACKET_MULTICAST;
    } else if (dst == Mac48Address::ConvertFrom (m_address))
    {
      type = NetDevice::PACKET_HOST;
    } else {
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      NS_ASSERT (!m_forwardUp.IsNull ());
      NS_LOG_DEBUG("Forward up: I am " << m_address << ". DST address is " << dst << ".");
      NS_LOG_DEBUG("Forward up packet UID: " << packet->GetUid());
      m_forwardUp (this, packet, llc.GetType (), from);
    }

  if (!m_promiscRxCallback.IsNull ())
    {
      NS_LOG_DEBUG("Promiscuous RX callback. Packet UID: " << packet->GetUid());
      m_promiscRxCallback (this, packet, llc.GetType (), from, dst, type);
    }
  else
  {
      NS_LOG_DEBUG("Promiscuous RX callback is not set");
  }
  return true;
}

bool
SatelliteNetDevice::IsMulticast () const
{
  return true;
}

bool
SatelliteNetDevice::IsBroadcast () const
{
  return true;
}

Address
SatelliteNetDevice::GetBroadcast () const
{
    return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
SatelliteNetDevice::IsLinkUp () const
{
  return m_linkUp;
}

bool
SatelliteNetDevice::SetMtu (const uint16_t mtu)
{
  m_mtu = mtu;
  return true;
}

uint32_t
SatelliteNetDevice::GetIfIndex () const
{
  return 0;
}


bool
SatelliteNetDevice::IsBridge () const
{
  return false;
}

bool
SatelliteNetDevice::IsPointToPoint () const
{
  return false;
}

void SatelliteNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this << &callback);
  m_linkChangeCallbacks.ConnectWithoutContext (callback);
}

void
SatelliteNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_channel = 0;
  m_queue = 0;
}

Time SatelliteNetDevice::GetInterframeGap ()
{
  return m_InterframeGap;
}

void SatelliteNetDevice::SetInterframeGap (Time &m_tInterframeGap)
{
  SatelliteNetDevice::m_InterframeGap = m_tInterframeGap;
}


std::vector<Ptr<NetDevice> > SatelliteNetDevice::GetCommunicationNeighbors () const
{
  //NS_LOG_FUNCTION (this);
  std::vector<Ptr<NetDevice> > neighbors;
  Time time = Time (0);
  //double time = Simulator::Now().GetSeconds();
  //We need to know current net device id, so:
  uint32_t currentIndex = 0;
  for (uint32_t i = 0; i < m_channel->GetNDevices (); i++)
    {
      if (m_channel->GetDevice (i)->GetAddress () == m_address)
        {
          currentIndex = i;
        }
    }
  //Searching for the nearest time in links:
  std::vector<bool> links;
  std::vector<SatelliteChannel::Links> all_links = m_channel->GetLinks ();

  //Below we try to find the closest time
  //ToDo: Binary search
  for (std::vector<SatelliteChannel::Links>::iterator it = all_links.begin (); it != all_links.end (); ++it) {
      if((it->m_time).Compare (time) > 0)
        {
          links = (it-1)->m_links.at (currentIndex);
          break;
        }
    }
  for (uint32_t i = 0; i < links.size (); i++)
    {
      if (i != currentIndex && links[i])
        {
          neighbors.push_back (m_channel->GetDevice (i));
        }
    }
  return neighbors;
}
}

