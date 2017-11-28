#include "sat-channel.h"
#include <ns3/simulator.h>
#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("ns3::SatChannel");

namespace ns3 {

    TypeId
    SatChannel::GetTypeId() {
        static TypeId tid = TypeId("ns3::SatChannel")
                .SetParent<Channel>()
                .SetGroupName("Channel")
                .AddAttribute ("Delay", "Propagation delay through the channel",
                               TimeValue (Seconds (0)),
                               MakeTimeAccessor (&SatChannel::m_delay),
                               MakeTimeChecker ());
        return tid;
    }

    SatChannel::SatChannel()
        :
            Channel(),
            m_delay(Seconds(0.)),
            m_nDevices(0) {
    }

    Ptr<NetDevice>
    SatChannel::GetDevice(uint32_t i) const {
        return Ptr<NetDevice>();
    }

    uint32_t
    SatChannel::GetNDevices() const {
        return m_nDevices;
    }

    bool
    SatChannel::TransmitStart (Ptr<const Packet> p, Ptr<SatNetDevice> src, Time txTime)
    {
        NS_LOG_FUNCTION (this << p << src);
        NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

        return true;
    }
}
