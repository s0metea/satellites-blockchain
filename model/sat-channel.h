#ifndef SAT_CHANNEL_H
#define SAT_CHANNEL_H

#include "sat-net-device.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/channel.h"
#include <ns3/nstime.h>

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

    /**
    * \returns the number of NetDevices connected to this Channel.
    */
    virtual uint32_t GetNDevices () const;

    /**
    * \param i index of NetDevice to retrieve
    * \returns one of the NetDevices connected to this channel.
    */
    virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:
    SatChannel (SatChannel const &);
    SatChannel& operator= (SatChannel const &);
    Time          m_delay;    //!< Propagation delay
    uint32_t       m_nDevices; //!< Devices of this channel
};
}

#endif //SAT_CHANNEL_H
