#include "sat-channel.h"
#include "sat-net-device.h"
#include <ns3/network-module.h>
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SatNetDevice");

NS_OBJECT_ENSURE_REGISTERED (SatNetDevice);

	TypeId
    SatNetDevice::GetTypeId ()
    {
		static TypeId tid = TypeId ("ns3::SatNetDevice")
		.SetParent<NetDevice> ()
		.SetGroupName ("NetDevice");
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
    }

    void
    SatNetDevice::SetIfIndex(const uint32_t index) {

    }

    Ptr<Channel>
    SatNetDevice::GetChannel(void) const {
        return m_channel;
    }

    void
    SatNetDevice::SetAddress(Address address) {

    }

    uint16_t
    SatNetDevice::GetMtu(void) const {
        return 0;
    }

    Address
    SatNetDevice::GetAddress(void) const
    {
        return Address();
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


        return true;
    }

    bool
    SatNetDevice::SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
        NS_ASSERT (Mac48Address::IsMatchingType (dest));
        NS_ASSERT (Mac48Address::IsMatchingType (source));
        LlcSnapHeader llc;
        llc.SetType (protocolNumber);
        packet->AddHeader (llc);


        return true;
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
        return false;
    }

    Address
    SatNetDevice::GetBroadcast(void) const
    {
        return Address();
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
        return false;
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
}
