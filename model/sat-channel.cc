#include "sat-channel.h"
#include "sat-net-device.h"
#include <ns3/nstime.h>

namespace ns3 {

    TypeId
    SatChannel::GetTypeId() {
        static TypeId tid = TypeId("ns3::SatChannel")
                .SetParent<Channel>()
                .SetGroupName("Channel");
        return tid;
    }

    SatChannel::SatChannel()
        :
            Channel(),
            m_delay(Seconds(0.)),
            m_nDevices(0) {
    }

    Ptr<NetDevice> SatChannel::GetDevice(uint32_t i) const {
        return Ptr<NetDevice>();
    }

    uint32_t SatChannel::GetNDevices() const {
        return m_nDevices;
    }
}
