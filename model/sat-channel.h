#ifndef SAT_CHANNEL_H
#define SAT_CHANNEL_H

#include "sat-net-device.h"
#include <ns3/node.h>
#include <ns3/channel.h>
#include <ns3/nstime.h>
#include <ns3/propagation-delay-model.h>

namespace ns3 {

class SatNetDevice;
class Packet;

class SatChannel: public Channel
{
public:

    /**
    * \brief Get the TypeId
    *
    * \return The TypeId for this class
    */
    static TypeId GetTypeId (void);

    /**
    * \brief Create a SatChannel
    *
    * By default, you get a channel that has an "infinitely" fast
    * transmission speed and zero delay.
    */
    SatChannel ();

    virtual ~SatChannel ();

    /**
    * \returns the number of NetDevices connected to this Channel.
    */
    virtual uint32_t GetNDevices () const;

    /**
    * \param i index of NetDevice to retrieve
    * \returns one of the NetDevices connected to this channel.
    */
    virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

    /**
    * Adds the given SatNetDevice to the NetDevice list
    *
    * \param device the SatNetDevice to be added to the SatNetDevice list
    */
    void Add (Ptr<SatNetDevice> device);

    /**
    * \param delay the new propagation delay.
    */
    void SetPropagationDelay (Ptr<PropagationDelayModel> delay);

    /**
   * \brief Transmit a packet over this channel
   * \param packet Packet to transmit
   * \param src Source SatNetDevice
   * \param txTime Transmit time to apply
   * \returns true if successful (currently always true)
   */
    virtual bool TransmitStart (Ptr<SatNetDevice> src, Ptr<const Packet> packet,  Time txTime);

private:
    SatChannel (SatChannel const &);
    SatChannel& operator= (SatChannel const &);
    Ptr<PropagationDelayModel> m_delay; //!< Propagation delay model
    uint32_t m_nDevices; //!< Devices of this channel
    typedef std::vector<Ptr<SatNetDevice>> NetList;
    NetList netDeviceList; //!< List of SatNetDevices connected to this SatNetChannel

    /**
    * This method is scheduled by TransmitStart for each associated SatNetDevice.
    *
    * \param device the device to which the packet is destined
    * \param packet the packet being sent
    * \param duration the transmission duration associated with the packet being sent
    */
    virtual void Receive(Ptr<SatNetDevice> device, Ptr<Packet> packet, Time duration);
};
}

#endif //SAT_CHANNEL_H
