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
                .SetParent<NetDevice> ()
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
                               MakeUintegerChecker<uint16_t> ())
                .AddAttribute ("Address",
                               "The MAC address of this device.",
                               Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                               MakeMac48AddressAccessor (&SatelliteNetDevice::m_address),
                               MakeMac48AddressChecker ())
                .AddAttribute ("InterframeGap",
                               "The time to wait between packet (frame) transmissions",
                               TimeValue (MicroSeconds (static_cast<uint64_t>(10.0))),
                               MakeTimeAccessor (&SatelliteNetDevice::m_tInterframeGap),
                               MakeTimeChecker ());
        return tid;
    }

    SatelliteNetDevice::SatelliteNetDevice():
            m_txMachineState (READY),
            m_channel (nullptr),
            m_linkUp (false),
            m_currentPkt (nullptr) {
    }

    SatelliteNetDevice::~SatelliteNetDevice()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    SatelliteNetDevice::SetIfIndex(const uint32_t index) {

    }

    void
    SatelliteNetDevice::SetDataRate(DataRate bps)
    {
        NS_LOG_FUNCTION (this);
        this->bps = bps;
    }

    Ptr<Channel>
    SatelliteNetDevice::GetChannel(void) const {
        return m_channel;
    }

    void
    SatelliteNetDevice::SetChannel(Ptr<SatelliteChannel> channel)  {
        NS_LOG_FUNCTION (this << channel);
        m_channel = channel;
        m_linkUp = true;
    }

    void
    SatelliteNetDevice::SetAddress(Address address) {
        NS_LOG_FUNCTION (this << address);
        m_address = Mac48Address::ConvertFrom(address);
    }

    uint16_t
    SatelliteNetDevice::GetMtu(void) const {
        NS_LOG_FUNCTION (this);
        return static_cast<uint16_t>(m_mtu);
    }

    Address
    SatelliteNetDevice::GetAddress(void) const
    {
        return m_address;
    }

    bool
    SatelliteNetDevice::SupportsSendFrom(void) const
    {
        return false;
    }

    void
    SatelliteNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
    {

    }

    Address
    SatelliteNetDevice::GetMulticast(Ipv6Address addr) const
    {
        return Address();
    }

    void
    SatelliteNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
    {
        m_forwardUp = cb;
    }

    bool
    SatelliteNetDevice::NeedsArp(void) const
    {
        return false;
    }

    void
    SatelliteNetDevice::SetNode(Ptr<Node> node)
    {
        m_node = node;
    }

    Ptr<Node>
    SatelliteNetDevice::GetNode(void) const
    {
        return m_node;
    }


    bool
    SatelliteNetDevice::Send(Ptr<Packet> packet, const Address &dst, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet);
        NS_ASSERT (Mac48Address::IsMatchingType (dst));
        m_queue->Enqueue (packet);

        NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
        m_txMachineState = BUSY;
        m_currentPkt = packet;

        Time txTime = bps.CalculateBytesTxTime (packet->GetSize ());
        Time txCompleteTime = txTime + m_tInterframeGap;

        NS_LOG_LOGIC ("Schedule TransmitComplete event in " << txCompleteTime.GetSeconds () << "sec");
        Simulator::ScheduleWithContext(this->GetChannel()->GetId(), txTime, &SatelliteChannel::Send, this->GetChannel(), packet, this->m_address, dest);
        Simulator::ScheduleWithContext(this->GetNode()->GetId(), txCompleteTime, &SatelliteNetDevice::TransmitComplete, this);
        return true;
    }

    bool
    SatelliteNetDevice::Receive (Ptr<Packet> packet, Address &from) {
        NS_LOG_FUNCTION (this << packet << from);
        Time txTime = bps.CalculateBytesTxTime (packet->GetSize ());
        Time txCompleteTime = txTime + m_tInterframeGap;
        Simulator::ScheduleWithContext(this->GetNode()->GetId(), txCompleteTime, &SatelliteNetDevice::m_forwardUp, this, packet);
        return true;
    }

    bool
    SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
    {
        return false;
    }

    void
    SatelliteNetDevice::TransmitComplete (void)
    {
        NS_LOG_FUNCTION (this);

        //
        // This function is called in moment we're all done transmitting a packet.
        // We try and pull another packet off of the transmit queue.  If the queue
        // is empty, we are done, otherwise we need to start transmitting the
        // next packet.m_address
        //
        NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
        m_txMachineState = READY;

        NS_ASSERT_MSG (m_currentPkt != nullptr, "SatelliteNetDevice::TransmitComplete()");

        m_currentPkt = nullptr;

        Ptr<Packet> p = m_queue->Dequeue ();
        if (p == nullptr) {
            NS_LOG_LOGIC ("No pending packets in device queue after tx complete");
        } else {
            NS_LOG_LOGIC (this << "Queue is not empty" << p->GetUid());
        }
    }

    Address
    SatelliteNetDevice::GetMulticast(Ipv4Address multicastGroup) const
    {
        return Address();
    }

    bool
    SatelliteNetDevice::IsMulticast(void) const
    {
        return false;
    }

    bool
    SatelliteNetDevice::IsBroadcast(void) const
    {
        return true;
    }

    Address
    SatelliteNetDevice::GetBroadcast(void) const
    {
        NS_LOG_FUNCTION (this);
        return Mac48Address ("ff:ff:ff:ff:ff:ff");
    }

    bool
    SatelliteNetDevice::IsLinkUp(void) const
    {
        NS_LOG_FUNCTION (this);
        return m_linkUp;
    }

    void
    SatelliteNetDevice::AddLinkChangeCallback(Callback<void> callback)
    {
        NS_LOG_FUNCTION (this << "Empty");

    }

    bool
    SatelliteNetDevice::SetMtu(const uint16_t mtu)
    {
        NS_LOG_FUNCTION (this << mtu);
        m_mtu = mtu;
        return true;
    }

    uint32_t
    SatelliteNetDevice::GetIfIndex(void) const
    {
        return 0;
    }


    bool
    SatelliteNetDevice::IsBridge(void) const
    {
        return false;
    }

    bool
    SatelliteNetDevice::IsPointToPoint(void) const
    {
        return false;
    }

    void
    SatelliteNetDevice::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_node = nullptr;
        m_channel = nullptr;
        m_currentPkt = nullptr;
        m_queue = nullptr;
        NetDevice::DoDispose ();
    }


}
