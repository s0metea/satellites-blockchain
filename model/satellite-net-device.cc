#include "satellite-net-device.h"
#include <ns3/network-module.h>
#include <ns3/pointer.h>
#include <ns3/drop-tail-queue.h>

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("ns3::SatelliteNetDevice");

    NS_OBJECT_ENSURE_REGISTERED (SatelliteNetDevice);

    TypeId
    SatelliteNetDevice::GetTypeId() {
        static TypeId tid = TypeId("ns3::SatelliteNetDevice")
                .SetParent<lrr::NeighborAwareDevice>()
                .SetGroupName("NetDevice")
                .AddAttribute("DataRate",
                              "The data rate that the Net Device uses to simulate packet transmission timing.",
                              DataRateValue(DataRate("1250MB/s")),
                              MakeDataRateAccessor(&SatelliteNetDevice::bps),
                              MakeDataRateChecker())
                .AddAttribute("Mtu", "The MAC-level Maximum Transmission Unit",
                              UintegerValue(DEFAULT_MTU),
                              MakeUintegerAccessor(&SatelliteNetDevice::SetMtu,
                                                   &SatelliteNetDevice::GetMtu),
                              MakeUintegerChecker<uint16_t>())
                .AddAttribute("InterframeGap",
                              "The time to wait between packet (frame) transmissions",
                              TimeValue(MicroSeconds(static_cast<uint64_t> (10.0))),
                              MakeTimeAccessor(&SatelliteNetDevice::m_InterframeGap),
                              MakeTimeChecker());
        return tid;
    }

    SatelliteNetDevice::SatelliteNetDevice()
            : m_txMachineState(READY),
              m_channel(nullptr),
              m_linkUp(false) {
        NS_LOG_FUNCTION (this);
        m_queue = CreateObject<DropTailQueue<Packet>>();
        m_forwardUp = MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address &>();
    }

    SatelliteNetDevice::~SatelliteNetDevice() {
        NS_LOG_FUNCTION (this);
    }

    void
    SatelliteNetDevice::SetIfIndex(const uint32_t index) {

    }

    void
    SatelliteNetDevice::SetDataRate(DataRate m_bps) {
        m_bps = bps;
    }

    Ptr<Queue<Packet> >
    SimpleNetDevice::GetQueue() const {
        return m_queue;
    }

    void
    SimpleNetDevice::SetQueue(Ptr<Queue<Packet> > q) {
        NS_LOG_FUNCTION (this << q);
        m_queue = q;
    }

    Ptr<Channel>
    SatelliteNetDevice::GetChannel(void) const {
        return m_channel;
    }

    void
    SatelliteNetDevice::SetChannel(Ptr<SatelliteChannel> channel) {
        m_channel = channel;
    }

    void
    SatelliteNetDevice::SetAddress(Address address) {
        m_address = Mac48Address::ConvertFrom(address);
    }

    Address
    SatelliteNetDevice::GetAddress(void) const {
        return m_address;
    }

    uint16_t
    SatelliteNetDevice::GetMtu(void) const {
        return static_cast<uint16_t> (m_mtu);
    }


    bool
    SatelliteNetDevice::SupportsSendFrom(void) const {
        return false;
    }

    void
    SatelliteNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) {
    }

    Address
    SatelliteNetDevice::GetMulticast(Ipv4Address multicastGroup) const {
        return Mac48Address::GetMulticast(multicastGroup);
    }

    Address
    SatelliteNetDevice::GetMulticast(Ipv6Address addr) const {
        return Mac48Address::GetMulticast(addr);
    }

    void
    SatelliteNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb) {
        m_forwardUp = cb;
    }

    bool
    SatelliteNetDevice::NeedsArp(void) const {
        return false;
    }

    void
    SatelliteNetDevice::SetNode(Ptr<Node> node) {
        m_node = node;
    }

    Ptr<Node>
    SatelliteNetDevice::GetNode(void) const {
        return m_node;
    }


    bool
    SatelliteNetDevice::Send(Ptr<Packet> packet, const Address &dst, uint16_t protocol) {
        m_protocol = protocol;

        //LLCHeader:
        LlcSnapHeader llc;
        llc.SetType(protocol);
        packet->AddHeader(llc);

        //EthernetHeader:
        EthernetHeader ethernetHeader;
        ethernetHeader.SetDestination(Mac48Address::ConvertFrom(dst));
        ethernetHeader.SetSource(Mac48Address::ConvertFrom(m_address));

        packet->AddHeader(ethernetHeader);

        m_queue->Enqueue(packet);

        if (m_txMachineState == READY) {
            TX();
        }
        return true;
    }

    bool
    SatelliteNetDevice::TX() {
        if (m_queue->GetNPackets() == 0) {
            m_txMachineState = READY;
            return true;
        }
        m_txMachineState = BUSY;
        NS_ASSERT(!m_txEvent.IsRunning());
        Ptr<Packet> m_currentPkt = m_queue->Dequeue();
        m_currentTxPacket = m_currentPkt;
        EthernetHeader ethernetHeader;
        m_currentPkt->PeekHeader(ethernetHeader);
        Time totalTime = m_InterframeGap + bps.CalculateBytesTxTime(m_currentPkt->GetSize());
        m_channel->Send(m_currentPkt, m_protocol, ethernetHeader.GetDestination(), this);
        m_txEvent = Simulator::Schedule(totalTime, &SatelliteNetDevice::TX, this);
        return true;
    }

    DataRate
    SatelliteNetDevice::GetDataRate() {
        return bps;
    }

    bool
    SatelliteNetDevice::StartRX(Ptr<Packet> packet, const Address &src, uint16_t protocol) {
        m_protocol = protocol;
        Time totalTime = m_InterframeGap + bps.CalculateBytesTxTime(packet->GetSize());
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
        NetDevice::PacketType type;
        if (dst.IsBroadcast()) {
            type = NetDevice::PACKET_BROADCAST;
        } else if (dst.IsGroup()) {
            type = NetDevice::PACKET_MULTICAST;
        } else if (dst == Mac48Address::ConvertFrom(m_address)) {
            type = NetDevice::PACKET_HOST;
        } else {
            type = NetDevice::PACKET_OTHERHOST;
        }

        if (type != NetDevice::PACKET_OTHERHOST) {
            packet->RemoveHeader(llc);
            NS_ASSERT (!m_forwardUp.IsNull());
            m_forwardUp(this, packet, llc.GetType(), from);
        }
        return true;
    }

    bool
    SatelliteNetDevice::IsMulticast() const {
        return true;
    }

    bool
    SatelliteNetDevice::IsBroadcast() const {
        return true;
    }

    Address
    SatelliteNetDevice::GetBroadcast() const {
        return Mac48Address::GetBroadcast();
    }

    bool
    SatelliteNetDevice::IsLinkUp() const {
        return m_linkUp;
    }

    bool
    SatelliteNetDevice::SetMtu(const uint16_t mtu) {
        m_mtu = mtu;
        return true;
    }

    uint32_t
    SatelliteNetDevice::GetIfIndex() const {
        return 0;
    }


    bool
    SatelliteNetDevice::IsBridge() const {
        return false;
    }

    bool
    SatelliteNetDevice::IsPointToPoint() const {
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
    }

    bool SatelliteNetDevice::SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                                      uint16_t protocolNumber) {
        return false;
    }

    Time SatelliteNetDevice::GetInterframeGap() {
        return m_InterframeGap;
    }

    void SatelliteNetDevice::SetInterframeGap(Time &m_tInterframeGap) {
        SatelliteNetDevice::m_InterframeGap = m_tInterframeGap;
    }


    std::vector<Ptr<NetDevice>> SatelliteNetDevice::GetCommunicationNeighbors() const {
        std::vector<Ptr<NetDevice>> neighbors;
        double time = 0.0;
        //We need to know current net device id, so:
        uint32_t currentIndex = 0;
        for (uint32_t i = 0; i < m_channel->GetNDevices(); i++) {
            if (m_channel->GetDevice(i)->GetAddress() == m_address)
                currentIndex = i;
        }
        //Choosing current time frame and current net device:
        std::vector<bool> links = m_channel->GetLinks().find(time)->second[currentIndex];
        for (uint32_t i = 0; i < links.size(); i++) {
            if (i != currentIndex && links[i] == true) {
                neighbors.push_back(m_channel->GetDevice(i));
            }
        }
        return neighbors;
    }
}

