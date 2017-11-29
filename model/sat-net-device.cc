#include "sat-channel.h"
#include <ns3/network-module.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ns3::SatNetDevice");

NS_OBJECT_ENSURE_REGISTERED (SatNetDevice);

	TypeId
    SatNetDevice::GetTypeId ()
    {
		static TypeId tid = TypeId ("ns3::SatNetDevice")
		.SetParent<NetDevice> ()
		.SetGroupName ("NetDevice")
        .AddAttribute ("DataRate",
                       "The data rate that the Net Device uses to simulate packet transmission timing.",
                       DataRateValue (DataRate ("1250MB/s")),
                       MakeDataRateAccessor (&SatNetDevice::bps),
                       MakeDataRateChecker ())
                .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                               UintegerValue (DEFAULT_MTU),
                               MakeUintegerAccessor (&SatNetDevice::SetMtu,
                                                     &SatNetDevice::GetMtu),
                               MakeUintegerChecker<uint16_t> ())
                .AddAttribute ("Address",
                               "The MAC address of this device.",
                               Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                               MakeMac48AddressAccessor (&SatNetDevice::m_address),
                               MakeMac48AddressChecker ())
                .AddAttribute ("InterframeGap",
                               "The time to wait between packet (frame) transmissions",
                               TimeValue (MicroSeconds (10.0)),
                               MakeTimeAccessor (&SatNetDevice::m_tInterframeGap),
                               MakeTimeChecker ());
        return tid;
	}

    SatNetDevice::SatNetDevice():
            m_txMachineState (READY),
            m_channel (0),
            m_linkUp (false),
            m_currentPkt (0) {
    }

    SatNetDevice::~SatNetDevice()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    SatNetDevice::SetIfIndex(const uint32_t index) {

    }

    void
    SatNetDevice::SetDataRate(DataRate bps)
    {
        NS_LOG_FUNCTION (this);
        this->bps = bps;
    }

    Ptr<Channel>
    SatNetDevice::GetChannel(void) const {
        return m_channel;
    }

    void
    SatNetDevice::SetAddress(Address address) {
        NS_LOG_FUNCTION (this << address);
        m_address = Mac48Address::ConvertFrom (address);
    }

    uint16_t
    SatNetDevice::GetMtu(void) const {
        NS_LOG_FUNCTION (this);
        return static_cast<uint16_t>(m_mtu);
    }

    Address
    SatNetDevice::GetAddress(void) const
    {
        return m_address;
    }

    bool
    SatNetDevice::SupportsSendFrom(void) const
    {
        return false;
    }

    void
    SatNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
    {

    }

    Address
    SatNetDevice::GetMulticast(Ipv6Address addr) const
    {
        return Address();
    }

    void
    SatNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
    {
        m_forwardUp = cb;
    }

    bool
    SatNetDevice::NeedsArp(void) const
    {
        return false;
    }

    void
    SatNetDevice::SetNode(Ptr<Node> node)
    {
        m_node = node;
    }

    Ptr<Node>
    SatNetDevice::GetNode(void) const
    {
        return m_node;
    }

    bool
    SatNetDevice::Send(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
        NS_ASSERT (Mac48Address::IsMatchingType (dest));

        LlcSnapHeader llc;
        llc.SetType (protocolNumber);
        packet->AddHeader (llc);

        bool ret = false;
        if (m_txMachineState == READY)
        {
            packet = m_queue->Dequeue ();
            ret = TransmitStart (packet);
        }
        return ret;
    }

    void
    SatNetDevice::TransmitComplete (void)
    {
        NS_LOG_FUNCTION (this);

        //
        // This function is called to when we're all done transmitting a packet.
        // We try and pull another packet off of the transmit queue.  If the queue
        // is empty, we are done, otherwise we need to start transmitting the
        // next packet.
        //
        NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
        m_txMachineState = READY;

        NS_ASSERT_MSG (m_currentPkt != 0, "SatNetDevice::TransmitComplete()");

        m_currentPkt = 0;

        Ptr<Packet> p = m_queue->Dequeue ();
        if (p == 0)
        {
            NS_LOG_LOGIC ("No pending packets in device queue after tx complete");
            return;
        }

        TransmitStart (p);
    }

    bool
    SatNetDevice::TransmitStart (Ptr<Packet> p)
    {
        NS_LOG_FUNCTION (this << p);
        NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

        //
        // This function is called to start the process of transmitting a packet.
        // We need to tell the channel that we've started wiggling the wire and
        // schedule an event that will be executed when the transmission is complete.
        //
        NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
        m_txMachineState = BUSY;
        m_currentPkt = p;

        Time txTime = m_bps.CalculateBytesTxTime (p->GetSize ());
        Time txCompleteTime = txTime + m_tInterframeGap;

        NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << txCompleteTime.GetSeconds () << "sec");
        Simulator::Schedule (txCompleteTime, &SatNetDevice::TransmitComplete, this);

        bool result = m_channel->TransmitStart (this, p, txTime);
        return result;
    }

    bool
    SatNetDevice::SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
        return false;
    }

    Address
    SatNetDevice::GetMulticast(Ipv4Address multicastGroup) const
    {
        return Address();
    }

    bool
    SatNetDevice::IsMulticast(void) const
    {
        return false;
    }

    bool
    SatNetDevice::IsBroadcast(void) const
    {
        return true;
    }

    Address
    SatNetDevice::GetBroadcast(void) const
    {
        NS_LOG_FUNCTION (this);
        return Mac48Address ("ff:ff:ff:ff:ff:ff");
    }

    bool
    SatNetDevice::IsLinkUp(void) const
    {
        NS_LOG_FUNCTION (this);
        return m_linkUp;
    }

    void
    SatNetDevice::AddLinkChangeCallback(Callback<void> callback)
    {

    }

    bool
    SatNetDevice::SetMtu(const uint16_t mtu)
    {
        NS_LOG_FUNCTION (this << mtu);
        m_mtu = mtu;
        return true;
    }

    uint32_t
    SatNetDevice::GetIfIndex(void) const
    {
        return 0;
    }


    bool
    SatNetDevice::IsBridge(void) const
    {
        return false;
    }

    bool
    SatNetDevice::IsPointToPoint(void) const
    {
        return false;
    }

    void
    SatNetDevice::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_node = 0;
        m_channel = 0;
        m_currentPkt = 0;
        m_queue = 0;
        NetDevice::DoDispose ();
    }
}
