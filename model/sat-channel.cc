#include <ns3/log-macros-disabled.h>
#include <ns3/nstime.h>
#include "sat-channel.h"
#include "sat-net-device.h"

namespace ns3 {

    TypeId
    SatChannel::GetTypeId() {
        static TypeId tid = TypeId("ns3::SatChannel")
                .SetParent<Channel>()
                .SetGroupName("Channel");
        return tid;
    }

    SatChannel::SatChannel() :
            Channel(),
            m_delay(Seconds(0.)),
            m_nDevices(0) {
        //NS_LOG_FUNCTION_NOARGS();
    }

    SatChannel::~SatChannel() {}

    uint32_t
    SatChannel::GetNDevices() {
        return 0;
    }

    Ptr<NetDevice>
    SatChannel::GetDevice(uint32_t i) {
        return Ptr<SatNetDevice>();
    };
}
