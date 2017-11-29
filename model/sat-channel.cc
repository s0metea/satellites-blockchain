#include "sat-channel.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/pointer.h>

NS_LOG_COMPONENT_DEFINE("ns3::SatChannel");

namespace ns3 {

    TypeId
    SatChannel::GetTypeId() {
        static TypeId tid = TypeId("ns3::SatChannel")
                .SetParent<Channel>()
                .SetGroupName("Channel")
                .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                               PointerValue (),
                               MakePointerAccessor (&SatChannel::m_delay),
                               MakePointerChecker<PropagationDelayModel> ());
        return tid;
    }

    SatChannel::SatChannel()
        :
            Channel(),
            m_nDevices(0) {
    }

    SatChannel::~SatChannel ()
    {
        NS_LOG_FUNCTION (this);
        netDeviceList.clear ();
    }

    Ptr<NetDevice>
    SatChannel::GetDevice(uint32_t i) const
    {
        return netDeviceList[i];
    }

    void
    SatChannel::Add(Ptr<SatNetDevice> device)
    {
        netDeviceList.push_back(device);
    }


    uint32_t
    SatChannel::GetNDevices() const {
        return m_nDevices;
    }

    void
    SatChannel::SetPropagationDelay(const Ptr<PropagationDelayModel> delay)
    {
        m_delay = delay;
    }

    bool
    SatChannel::TransmitStart (Ptr<SatNetDevice> src, Ptr<const Packet> packet, Time txTime)
    {
        NS_LOG_FUNCTION (this << packet << src);
        NS_LOG_LOGIC ("UID is " << packet->GetUid () << ")");

        Ptr<MobilityModel> senderMobility = src->GetNode()->GetObject<MobilityModel> ();
        NS_ASSERT (senderMobility != 0);

        for (NetList::const_iterator i = netDeviceList.begin (); i != netDeviceList.end (); i++)
        {
            if (src != (*i))
            {
                Ptr<MobilityModel> receiverMobility = (*i)->GetNode()->GetObject<MobilityModel> ();
                Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
                NS_LOG_DEBUG ("Propagation Delay =" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);
                Ptr<Packet> copy = packet->Copy ();
                Ptr<SatNetDevice> dstNetDevice = *i;
                uint32_t dstNode;
                dstNode = dstNetDevice->GetNode ()->GetId();
                Simulator::ScheduleWithContext (dstNode,
                                                delay, &SatChannel::Receive,
                                                (*i), copy, txTime);
            }
        }
        return true;
    }


    void
    SatChannel::Receive (Ptr<SatNetDevice> device, Ptr<Packet> packet, Time duration)
    {
        NS_LOG_FUNCTION (device << packet << duration.GetSeconds ());
    }
}
