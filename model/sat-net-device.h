#ifndef SAT_NETDEVICE_H
#define SAT_NETDEVICE_H

#include <ns3/data-rate.h>
#include "sat-channel.h"
#include "ns3/net-device.h"
#include "ns3/log.h"

namespace ns3 {

template <typename Item> class Queue;

class SatChannel;

class SatNetDevice: public NetDevice
{
public:
    /**
    * \brief Get the TypeId
    *
    * \return The TypeId for this class
    */
    static TypeId GetTypeId ();

    SatNetDevice ();

    virtual ~SatNetDevice ();

    //inherited from NetDevice base class.
    void SetIfIndex (const uint32_t index);
    uint32_t GetIfIndex (void) const;

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

    bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
    bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

    Ptr<Node> GetNode (void) const;
    void SetNode (Ptr<Node> node);

    bool NeedsArp (void) const;

    void SetReceiveCallback (NetDevice::ReceiveCallback cb);

    Address GetMulticast (Ipv6Address addr) const;

    void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
    bool SupportsSendFrom (void) const;

    void SatNetDevice::SetDataRate (DataRate bps);

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
   * Start Sending a Packet Down the Wire.
   *
   * The TransmitStart method is the method that is used internally in the
   * SatNetDevice to begin the process of sending a packet out on
   * the channel.  The corresponding method is called on the channel to let
   * it know that the physical device this class represents has virtually
   * started sending signals.  An event is scheduled for the time at which
   * the bits have been completely transmitted.
   *
   * \see SatNetDevice::TransmitStart ()
   * \see TransmitComplete()
   * \param p a reference to the packet to send
   * \returns true if success, false on failure
   */
    bool TransmitStart (Ptr<Packet> p);

    /**
     * Stop Sending a Packet Down the Wire and Begin the Interframe Gap.
     *
     * The TransmitComplete method is used internally to finish the process
     * of sending a packet out on the channel.
     */
    void TransmitComplete (void);

    /**
     * \brief Make the link up and running
     *
     * It calls also the linkChange callback.
     */

    /**
    * The state of the Net Device transmit state machine.
    * \see TxMachineState
    */
    TxMachineState m_txMachineState;

    /**
    * The data rate that the Net Device uses to simulate packet transmission
    * timing.
    * \see class DataRate
    */
    DataRate m_bps;
    Ptr<Node> m_node; //!< Node owning this NetDevice
    Mac48Address m_address;
    Ptr<SatChannel> m_channel;
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
    * The Queue which this SatNetDevice uses as a packet source.
    * Management of this Queue has been delegated to the SatNetDevice
    * and it has the responsibility for deletion.
    * \see class DropTailQueue
    */
    Ptr<Queue<Packet> > m_queue;


    NetDevice::ReceiveCallback m_forwardUp; //!< forward up callback

    /**
    * \brief Copy constructor
    *
    * The method is private, so it is DISABLED.
    * \param o Other NetDevice
    */
    SatNetDevice (const SatNetDevice  &o);

    /**
    * \brief Assign operator
    *
    * The method is private, so it is DISABLED.
    *
    * \param o Other NetDevice
    * \return New instance of the NetDevice
    */
    SatNetDevice& operator= (const SatNetDevice &o);
};

}

#endif //SAT_NETDEVICE_H
