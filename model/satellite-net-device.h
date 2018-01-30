#ifndef SAT_NETDEVICE_H
#define SAT_NETDEVICE_H

#include <ns3/data-rate.h>
#include <ns3/satellite-channel.h>
#include <ns3/event-id.h>
#include <ns3/net-device.h>
#include <ns3/lrr-device.h>
#include <ns3/callback.h>
#include <ns3/traced-callback.h>

namespace ns3 {

template <typename Item>
class DropTailQueue;
class NetDevice;

class SatelliteNetDevice : public lrr::NeighborAwareDevice
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

  void SetChannel (Ptr<SatelliteChannel> channel);

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

  bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocol);

  bool SendFrom (Ptr<Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber);

  Ptr<Node> GetNode (void) const;
  void SetNode (Ptr<Node> node);

  bool NeedsArp (void) const;
  Address GetMulticast (Ipv6Address addr) const;

  void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);

  bool SupportsSendFrom (void) const;

  void SetDataRate (DataRate m_bps);

  std::vector<Ptr<NetDevice> > GetCommunicationNeighbors () const;

  /**
* \brief Dispose of the object
*/
  void DoDispose ();

  bool StartRX (Ptr<Packet> packet);

  void SetInterframeGap (Time &m_tInterframeGap);

  Time GetInterframeGap ();
  DataRate GetDataRate ();

private:
  /**
  * Enumeration of the states of the transmit machine of the net device.
  */
  enum TxMachineState
  {
    READY,       /**< The transmitter is ready to begin transmission of a packet */
    BUSY         /**< The transmitter is busy transmitting a packet */
  };

  bool TX ();

  bool ForwardUp (Ptr<Packet> packet);

  /**
  * The state of the Net Device transmit state machine.
  * \see TxMachineState
  */
  TxMachineState m_txMachineState;
  EventId m_txEvent;

  DataRate bps;

  /**
  * The interfrace gap that the Net Device uses to throttle packet
  * transmission
  */
  Time m_InterframeGap;
  Ptr<Packet> m_currentTxPacket;
  Ptr<Node> m_node;   //!< Node owning this NetDevice
  Address m_address;
  Ptr<SatelliteChannel> m_channel;
  bool m_linkUp;        //!< Identify if the link is up or not
  static const uint16_t DEFAULT_MTU = 1500;   //!< Default MTU

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
  Ptr<DropTailQueue<Packet> > m_queue;

  NetDevice::ReceiveCallback m_forwardUp;   //!< forward up callback

  /**
  * The callback used to notify higher layers that a packet has been received in promiscuous mode.
  */
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  /**
  * List of callbacks to fire if the link changes state (up or down).
  */
  TracedCallback<> m_linkChangeCallbacks;

  /**
  * \brief Copy constructor
  *
  * The method is private, so it is DISABLED.
  * \param o Other NetDevice
  */
  SatelliteNetDevice (const SatelliteNetDevice &o);

  /**
  * \brief Assign operator
  *
  * The method is private, so it is DISABLED.
  *
  * \param o Other NetDevice
  * \return New instance of the NetDevice
  */
  SatelliteNetDevice &operator= (const SatelliteNetDevice &o);
};

} //ns3 namespace

#endif //SAT_NETDEVICE_H
