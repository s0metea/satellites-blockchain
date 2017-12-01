#ifndef SAT_NETDEVICE_H
#define SAT_NETDEVICE_H

#include <ns3/data-rate.h>
#include <ns3/satellite-channel.h>
#include "ns3/queue.h"
#include "ns3/net-device.h"


namespace ns3 {

    template <typename Item> class Queue;
    class NetDevice;

    class SatelliteNetDevice: public NetDevice
    {
    public:
        /**
        * \brief Get the TypeId
        *
        * \return The TypeId for this class
        */
        static TypeId GetTypeId ();

        SatelliteNetDevice ();

        virtual ~SatelliteNetDevice ();

        //inherited from NetDevice base class.
        void SetIfIndex (const uint32_t index);
        uint32_t GetIfIndex (void) const;

        void SetChannel(Ptr<SatelliteChannel> channel);
        Ptr<Channel> GetChannel (void) const;

        void SetAddress (Address address);
        Address GetAddress (void) const;

        bool SetMtu (const uint16_t mtu);
        uint16_t GetMtu (void) const;

        bool IsLinkUp (void) const;
        void AddLinkChangeCallback (Callback<void> callback);

        bool IsBroadcast (void) const;
        Address GetBroadcast (void) const;

        bool IsMulticast (void) const;
        Address GetMulticast (Ipv4Address multicastGroup) const;

        bool IsPointToPoint (void) const;
        bool IsBridge (void) const;

        bool Send (Ptr<Packet> packet, const Address&, uint16_t protocolNumber);
        bool Receive (Ptr<Packet> packet, Address &from);

        Ptr<Node> GetNode (void) const;

        void SetNode (Ptr<Node> node);

        bool NeedsArp (void) const;

        void SetReceiveCallback (NetDevice::ReceiveCallback cb);

        Address GetMulticast (Ipv6Address addr) const;

        void SetPromiscReceiveCallback (PromiscReceiveCallback cb);

        bool SupportsSendFrom (void) const;

        void SetDataRate (DataRate bps);

        /**
        * \brief Dispose of the object
        */
        virtual void DoDispose (void);

    private:

        /**
        * Enumeration of the states of the transmit machine of the net device.
        */
        enum TxMachineState
        {
            READY,   /**< The transmitter is ready to begin transmission of a packet */
            BUSY     /**< The transmitter is busy transmitting a packet */
        };


        /**
         * Stop Sending a Packet and Begin the Interframe Gap.
         *
         * The TransmitComplete method is used internally to finish the process
         * of sending a packet out on the channel.
         */
        void TransmitComplete (void);


        /**
        * The state of the Net Device transmit state machine.
        * \see TxMachineState
        */
        TxMachineState m_txMachineState;

        /**
        * The data rate in bits per second that the Net Device uses to simulate packet transmission
        * timing.bps
        * \see class DataRate
        */
        DataRate bps;

        /**
        * The interframe gap that the Net Device uses to throttle packet
        * transmission
        */
        Time m_tInterframeGap;
        Ptr<Node> m_node; //!< Node owning this NetDevice
        Address m_address;   //!< Mac48Address of this NetDevice
        Ptr<SatelliteChannel> m_channel;
        bool m_linkUp;      //!< Identify if the link is up or not
        Ptr<Packet> m_currentPkt; //!< Current packet processed
        static const uint16_t DEFAULT_MTU = 1500; //!< Default MTU

        /**
         * \brief The Maximum Transmission Unit
         *
         * This corresponds to the maximum
         * number of bytes that can be transmitted as seen from higher layers.
         * This corresponds to the 1500 byte MTU size often seen on IP over
         * Ethernet.
         */
        uint32_t m_mtu;

        /**
        * The Queue which this SatelliteNetDevice uses as a packet source.
        * Management of this Queue has been delegated to the SatNetDevice
        * and it has the responsibility for deletion.
        * \see class DropTailQueue
        */
        Ptr<Queue<Packet>> m_queue;

        ReceiveCallback m_forwardUp; //!< forward up callback

        /**
        * \brief Copy constructor
        *
        * The method is private, so it is DISABLED.
        * \param o Other NetDevice
        */
        SatelliteNetDevice (const SatelliteNetDevice  &o);

        /**
        * \brief Assign operator
        *
        * The method is private, so it is DISABLED.
        *
        * \param o Other NetDevice
        * \return New instance of the NetDevice
        */
        SatelliteNetDevice& operator= (const SatelliteNetDevice &o);

    };

}

#endif //SAT_NETDEVICE_H
