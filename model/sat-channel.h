#ifndef SAT_CHANNEL_H
#define SAT_CHANNEL_H

#include <ns3/nstime.h>
#include "ns3/node.h"
#include "ns3/channel.h"

namespace ns3 {

class SatChannel: public Channel {
public:
    static TypeId GetTypeId ();
    SatChannel ();
    ~SatChannel ();
    /**
    * \returns the number of NetDevices connected to this Channel.
    */
    uint32_t
    GetNDevices ();
    /**
    * \param i index of NetDevice to retrieve
    * \returns one of the NetDevices connected to this channel.
    */
    Ptr<NetDevice>
    GetDevice (uint32_t i);
private:
    SatChannel (SatChannel const &);
    SatChannel& operator= (SatChannel const &);
    Time          m_delay;    //!< Propagation delay
    int32_t       m_nDevices; //!< Devices of this channel
};
}

#endif //SAT_CHANNEL_H
