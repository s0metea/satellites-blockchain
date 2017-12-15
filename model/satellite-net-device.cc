#include "satellite-net-device.h"
#include <ns3/network-module.h>
#include <ns3/pointer.h>
#include <ns3/drop-tail-queue.h>

using namespace std;

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
//                .AddAttribute ("Address",
//                               "The MAC address of this device.",
//                               AddressValue (Mac48Address::Allocate()),
//                               MakeAddressAccessor (&SatelliteNetDevice::m_address),
//                               MakeAddressChecker ())
                .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                               UintegerValue (DEFAULT_MTU),
                               MakeUintegerAccessor (&SatelliteNetDevice::SetMtu,
                                                     &SatelliteNetDevice::GetMtu),
                               MakeUintegerChecker<uint16_t> ())
                .AddAttribute ("InterframeGap",
                               "The time to wait between packet (frame) transmissions",
                               TimeValue (MicroSeconds (static_cast<uint64_t>(10.0))),
                               MakeTimeAccessor (&SatelliteNetDevice::m_InterframeGap),
                               MakeTimeChecker ());
        return tid;
    }

    SatelliteNetDevice::SatelliteNetDevice():
            m_txMachineState (READY),
            m_channel (nullptr),
            totalTxSeconds(0),
            totalRxSeconds(0),
            m_linkUp (false) {
        NS_LOG_FUNCTION (this);
        m_queue = CreateObject<DropTailQueue<Packet> >();
        m_address = Mac48Address::Allocate ();
        //cout << this << "MAC Address: " << m_address << endl;
        m_forwardUp = MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address &>();
    }

    const Time SatelliteNetDevice::getTotalTxSeconds() {
        return totalTxSeconds;
    }

    const Time SatelliteNetDevice::getTotalRxSeconds() {
        return totalRxSeconds;
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
        this->bps = bps;
    }

    Ptr<Queue<Packet>>
    SimpleNetDevice::GetQueue () const
    {
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
        m_channel = channel;
    }

    void
    SatelliteNetDevice::SetAddress(Address address) {
        m_address = Mac48Address::ConvertFrom(address);
        cout << "MAC Address was set: " << m_address << endl;
    }

    Address
    SatelliteNetDevice::GetAddress(void) const
    {
        return m_address ;
    }

    uint16_t
    SatelliteNetDevice::GetMtu(void) const {
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
        return;
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
    }

    Ptr<Node>
    SatelliteNetDevice::GetNode(void) const
    {
        return m_node;
    }


    bool
    SatelliteNetDevice::Send(Ptr<Packet> packet, const Address &dst, uint16_t protocol)
    {
        m_protocol = protocol;

        //LLCHeader:
        LlcSnapHeader llc;
        llc.SetType (protocol);
        packet->AddHeader (llc);

        //EthernetHeader:
        EthernetHeader ethernetHeader;
        ethernetHeader.SetDestination(Mac48Address::ConvertFrom(dst));
        ethernetHeader.SetSource(Mac48Address::ConvertFrom(m_address));

        packet->AddHeader (ethernetHeader);

        m_queue->Enqueue(packet);

        if(m_txMachineState == READY) {
            TX();
        }
        else {
            Time txTime = bps.CalculateBytesTxTime (packet->GetSize());
            Time totalTime = txTime + m_InterframeGap;
            Simulator::Schedule(totalTime, &SatelliteNetDevice::TX, this);
        }
        return true;
    }

    bool
    SatelliteNetDevice::TX() {
        m_txMachineState = BUSY;
        if(m_queue->GetNPackets() > 0) {
            Ptr<Packet> m_currentPkt = m_queue->Dequeue();
            m_currentTxPacket = m_currentPkt;
            EthernetHeader ethernetHeader;
            m_currentPkt->PeekHeader(ethernetHeader);
            Time totalTime = m_InterframeGap + bps.CalculateBytesTxTime(m_currentPkt->GetSize());
            totalTxSeconds += totalTime;
            this->m_channel->Send(m_currentPkt, m_protocol, ethernetHeader.GetDestination(), this);
            Simulator::Schedule(totalTime, &SatelliteNetDevice::TX, this);
        } else
            m_txMachineState = READY;
        return true;
    }

    bool
    SatelliteNetDevice::StartRX(Ptr<Packet> packet, const Address &src, uint16_t protocol)
    {
        m_protocol = protocol;
        Time totalTime = m_InterframeGap + bps.CalculateBytesTxTime (packet->GetSize());
        totalRxSeconds += totalTime;
        Simulator::Schedule(totalTime, &SatelliteNetDevice::ForwardUp, this, packet);
        return true;
    }


    bool SatelliteNetDevice::ForwardUp(Ptr<Packet> packet) {

        EthernetHeader eh;
        LlcSnapHeader llc;

        packet->PeekHeader(eh);
        packet->PeekHeader(llc);
        Mac48Address from = eh.GetSource();
        Mac48Address dst = eh.GetDestination();
        packet->RemoveHeader(eh);
        //cout << dst << " " << Mac48Address::ConvertFrom(m_address) << endl;

        NetDevice::PacketType type;
        if (dst.IsBroadcast ())
        {
            type = NetDevice::PACKET_BROADCAST;
        }
        else if (dst.IsGroup ())
        {
            type = NetDevice::PACKET_MULTICAST;
        }
        else if (dst == Mac48Address::ConvertFrom(m_address))
        {
            type = NetDevice::PACKET_HOST;
        }
        else
        {
            type = NetDevice::PACKET_OTHERHOST;
        }

        if (type != NetDevice::PACKET_OTHERHOST)
        {
            packet->RemoveHeader (llc);
            NS_ASSERT(!m_forwardUp.IsNull());
            m_forwardUp(this, packet, llc.GetType(), from);
        }
        return true;
    }

    bool
    SatelliteNetDevice::IsMulticast(void) const {
        return true;
    }

    bool
    SatelliteNetDevice::IsBroadcast(void) const {
        return true;
    }

    Address
    SatelliteNetDevice::GetBroadcast(void) const {
        return Mac48Address::GetBroadcast();
    }

    bool
    SatelliteNetDevice::IsLinkUp(void) const {
        return m_linkUp;
    }

    bool
    SatelliteNetDevice::SetMtu(const uint16_t mtu) {
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
        m_queue = 0;
        NetDevice::DoDispose();
    }

    bool SatelliteNetDevice::SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                                      uint16_t protocolNumber) {
        return false;
    }

    Time SatelliteNetDevice::GetInterframeGap(){
        return m_InterframeGap;
    }

    void SatelliteNetDevice::SetInterframeGap(Time &m_tInterframeGap) {
        SatelliteNetDevice::m_InterframeGap = m_tInterframeGap;
    }

}

