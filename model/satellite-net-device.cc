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
                .AddAttribute ("Address",
                               "The MAC address of this device.",
                               AddressValue (Mac48Address::Allocate()),
                               MakeAddressAccessor (&SatelliteNetDevice::m_address),
                               MakeAddressChecker ())
                .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                               UintegerValue (DEFAULT_MTU),
                               MakeUintegerAccessor (&SatelliteNetDevice::SetMtu,
                                                     &SatelliteNetDevice::GetMtu),
                               MakeUintegerChecker<uint16_t> ())
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
    SatelliteNetDevice::SetIfIndex(const uint32_t index)
    {

    }

    void
    SatelliteNetDevice::SetDataRate(DataRate bps)
    {
        NS_LOG_FUNCTION (this);
        this->bps = bps;
    }

    Ptr<Queue<Packet>>
    SimpleNetDevice::GetQueue () const
    {
        NS_LOG_FUNCTION (this);
        return m_queue;
    }

    void
    SimpleNetDevice::SetQueue (Ptr<Queue<Packet>> q)
    {
        NS_LOG_FUNCTION (this << q);
        m_queue = q;
    }

    Ptr<Channel>
    SatelliteNetDevice::GetChannel(void) const {
        return m_channel;
    }

    void
    SatelliteNetDevice::SetChannel(Ptr<SatelliteChannel> channel)  {
        NS_LOG_FUNCTION (this << channel);
        m_channel = channel;
    }

    void
    SatelliteNetDevice::SetAddress(Address address) {
        m_address = Mac48Address::ConvertFrom(address);
        NS_LOG_FUNCTION (this << m_address);
    }

    Address
    SatelliteNetDevice::GetAddress(void) const
    {
        return m_address ;
    }

    uint16_t
    SatelliteNetDevice::GetMtu(void) const {
        NS_LOG_FUNCTION (this);
        return static_cast<uint16_t>(m_mtu);
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
    SatelliteNetDevice::GetMulticast(Ipv4Address multicastGroup) const
    {
        return Mac48Address::GetMulticast(multicastGroup);
    }

    Address
    SatelliteNetDevice::GetMulticast(Ipv6Address addr) const
    {
        return Mac48Address::GetMulticast(addr);
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
        node->AddDevice(this);
    }

    Ptr<Node>
    SatelliteNetDevice::GetNode(void) const
    {
        return m_node;
    }


    bool
    SatelliteNetDevice::Send(Ptr<Packet> packet, const Address &dst, uint16_t protocol)
    {
        NS_LOG_FUNCTION (this << packet);
        NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
        m_txMachineState = READY;
        m_currentPkt = packet;
        m_address = dst;
        m_protocol = protocol;
        m_queue->Enqueue(packet);
        Time txTime = bps.CalculateBytesTxTime (packet->GetSize());
        NS_LOG_INFO ("m_tInterframeGap: " << m_tInterframeGap);
        NS_LOG_INFO ("TX time: " << txTime);
        Time totalTime = txTime + m_tInterframeGap;
        Simulator::ScheduleWithContext(this->GetNode()->GetId(), totalTime, &SatelliteNetDevice::TX, this);
        return true;
    }

    bool
    SatelliteNetDevice::TX() {
        NS_LOG_FUNCTION (this);
        NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
        m_txMachineState = BUSY;
        Time totalTime = m_tInterframeGap;
        if(m_queue->GetNPackets() > 0) {
            this->m_channel->Send(m_queue->Dequeue(), m_protocol, m_address, this);
            m_currentPkt = nullptr;
            totalTime += bps.CalculateBytesTxTime (m_currentPkt->GetSize());
            Simulator::ScheduleWithContext(this->GetNode()->GetId(), totalTime, &SatelliteNetDevice::TX, this);
            return true;
        }
        return false;
    }

    bool
    SatelliteNetDevice::StartRX(Ptr<Packet> packet, const Address &src, uint16_t protocol)
    {
        NS_LOG_FUNCTION (this << packet);
        Time totalTime = m_tInterframeGap;
        if(m_queue->GetNPackets() > 0) {
            totalTime += bps.CalculateBytesTxTime (packet->GetSize());
            m_queue->Enqueue(packet);
            m_currentPkt = packet;
            m_address = src;
            m_protocol = protocol;
            Simulator::ScheduleWithContext(this->GetNode()->GetId(), totalTime, &SatelliteNetDevice::RX, this);
            return true;
        }
        return false;
    }

    bool SatelliteNetDevice::RX() {
        NS_LOG_FUNCTION (this);
        Time totalTime = m_tInterframeGap;
        if(m_queue->GetNPackets() > 0) {
            m_currentPkt = m_queue->Dequeue();
            totalTime += bps.CalculateBytesTxTime (m_currentPkt->GetSize());
            Simulator::ScheduleWithContext(this->GetNode()->GetId(), totalTime, &SatelliteNetDevice::RX, this);
            return true;
        }
        Simulator::ScheduleWithContext(this->GetNode()->GetId(), totalTime, &SatelliteNetDevice::ForwardUp, this);
        return false;
    }

    bool SatelliteNetDevice::ForwardUp() {
        m_forwardUp (this, m_currentPkt, m_protocol, m_address);
        return true;
    }

    bool
    SatelliteNetDevice::IsMulticast(void) const {
        return false;
    }

    bool
    SatelliteNetDevice::IsBroadcast(void) const {
        return true;
    }

    Address
    SatelliteNetDevice::GetBroadcast(void) const {
        NS_LOG_FUNCTION (this);
        return Mac48Address::GetBroadcast();
    }

    bool
    SatelliteNetDevice::IsLinkUp(void) const {
        NS_LOG_FUNCTION (this);
        return m_linkUp;
    }

    bool
    SatelliteNetDevice::SetMtu(const uint16_t mtu) {
        NS_LOG_FUNCTION (this << mtu);
        m_mtu = mtu;
        return true;
    }

    uint32_t
    SatelliteNetDevice::GetIfIndex(void) const {
        return 0;
    }


    bool
    SatelliteNetDevice::IsBridge(void) const {
        return false;
    }

    bool
    SatelliteNetDevice::IsPointToPoint(void) const {
        return false;
    }

    void SatelliteNetDevice::AddLinkChangeCallback(Callback<void> callback) {

    }

    void
    SatelliteNetDevice::DoDispose() {
        NS_LOG_FUNCTION (this);
        m_node = 0;
        m_channel = 0;
        m_currentPkt = 0;
        m_queue = 0;
        NetDevice::DoDispose();
    }

    bool SatelliteNetDevice::SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                                      uint16_t protocolNumber) {
        return false;
    }
}

