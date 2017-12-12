#include <ns3/simulator.h>
#include <ns3/mobility-model.h>
#include <ns3/pointer.h>
#include <ns3/log.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/satellite-net-device.h>

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("ns3::SatelliteChannel");

    NS_OBJECT_ENSURE_REGISTERED (SatelliteChannel);

    TypeId
    SatelliteChannel::GetTypeId() {
        static TypeId tid = TypeId("ns3::SatelliteChannel")
                .SetParent<Channel>()
                .SetGroupName("Channel")
                .AddAttribute("PropagationDelayModel",
                              "A pointer to the propagation delay model attached to this channel.",
                              PointerValue(),
                              MakePointerAccessor(&SatelliteChannel::m_delay),
                              MakePointerChecker<PropagationDelayModel>());
        return tid;
    }

    SatelliteChannel::SatelliteChannel() {
        NS_LOG_FUNCTION (this);
    }

    SatelliteChannel::~SatelliteChannel() {
        NS_LOG_FUNCTION (this);
        netDeviceList.clear();
    }

    Ptr<NetDevice>
    SatelliteChannel::GetDevice(uint32_t i) const {
        return netDeviceList[i];
    }

    void
    SatelliteChannel::Add(Ptr<SatelliteNetDevice> device) {
        NS_LOG_FUNCTION (this);
        netDeviceList.push_back(device);
    }

    uint32_t
    SatelliteChannel::GetNDevices() const {
        return static_cast<uint32_t>(netDeviceList.size());
    }

    void
    SatelliteChannel::SetPropagationDelay(const Ptr<PropagationDelayModel> delay) {
        m_delay = delay;
    }

    void
    SatelliteChannel::Send(Ptr<Packet> packet, uint16_t protocol, Address to, Ptr<SatelliteNetDevice> sender) {
        NS_LOG_FUNCTION (this << packet << sender);
        NS_LOG_LOGIC ("UID is " << packet->GetUid());
        Ptr<MobilityModel> senderMobility = sender->GetNode()->GetObject<MobilityModel>();
        NS_ASSERT (senderMobility != nullptr);
        for (auto netDevice = netDeviceList.begin(); netDevice != netDeviceList.end(); netDevice++) {
            if (sender->GetAddress() != (*netDevice)->GetAddress()) {
                Ptr<MobilityModel> receiverMobility = (*netDevice)->GetNode()->GetObject<MobilityModel>();
                Time delay = m_delay->GetDelay(senderMobility, receiverMobility);
                NS_LOG_DEBUG ("Propagation Delay =" << senderMobility->GetDistanceFrom(receiverMobility) << "m, delay="
                                                    << delay);
                Simulator::Schedule(delay, &SatelliteNetDevice::StartRX, (*netDevice), packet->Copy(), to, protocol);
            }
        }
    }
}