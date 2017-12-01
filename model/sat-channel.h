#ifndef SAT_CHANNEL_H
#define SAT_CHANNEL_H

#include "satellite-net-device.h"
#include <ns3/node.h>
#include <ns3/channel.h>
#include <ns3/nstime.h>
#include <ns3/propagation-delay-model.h>

namespace ns3 {

    class SatelliteChannel: public Channel
    {
    public:

        /**
        * \brief Get the TypeId
        *
        * \return The TypeId for this class
        */
        static TypeId GetTypeId (void);

        /**
        * \brief Create a SatelliteChannel
        *
        * By default, you get a channel that has an "infinitely" fast
        * transmission speed and zero delay.
        */
        SatelliteChannel ();

        virtual ~SatelliteChannel ();

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
        void Add(Ptr<SatelliteNetDevice> device);

        /**
        * \param delay the new propagation delay.
        */
        void SetPropagationDelay (Ptr<PropagationDelayModel> delay);

        /**
        * The channel
        * attempts to deliver the packet to all other SatelliteNetDevice objects
        * on the channel (except for the sender).
        *
        * \param packet the packet to send
        *\\param protocol
        *\\param to
        *\\param sender
        */
        void Send(Ptr<Packet> packet, uint16_t protocol, Address &to, Ptr<SatelliteNetDevice> sender);

    private:
        SatelliteChannel (SatelliteChannel const &);
        SatelliteChannel& operator= (SatelliteChannel const &);
        Ptr<PropagationDelayModel> m_delay; //!< Propagation delay model
        typedef std::vector<Ptr<SatelliteNetDevice>> NetList;
        NetList netDeviceList; //!< List of SatNetDevices connected to this SatNetChannel
    };
}

#endif //SAT_CHANNEL_H
