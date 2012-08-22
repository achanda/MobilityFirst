#ifndef BYPASS_H
#define BYPASS_H

#include <stdint.h>

#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ipv4-l4-protocol.h"

namespace ns3
{
	class Node;
	class Socket;
	//class BypassHeader;
	class Ipv4EndPointDemux;
	class Ipv4EndPoint;
	class BypassSocketImpl;
	
	class BypassProtocol : public Ipv4L4Protocol
	{
		public:
		static TypeId GetTypeId (void);
  static const uint8_t PROT_NUMBER;

  BypassProtocol ();
  virtual ~BypassProtocol ();

  void SetNode (Ptr<Node> node);

  virtual int GetProtocolNumber (void) const;

  /**
   * \return A smart Socket pointer to a UdpSocket, allocated by this instance
   * of the UDP protocol
   */
  Ptr<Socket> CreateSocket (void);

  Ipv4EndPoint *Allocate (void);
  Ipv4EndPoint *Allocate (Ipv4Address address);
  Ipv4EndPoint *Allocate (uint16_t port);
  Ipv4EndPoint *Allocate (Ipv4Address address, uint16_t port);
  Ipv4EndPoint *Allocate (Ipv4Address localAddress, uint16_t localPort,
                          Ipv4Address peerAddress, uint16_t peerPort);

  void DeAllocate (Ipv4EndPoint *endPoint);

  // called by UdpSocket.
  /**
   * \brief Send a packet via UDP
   * \param packet The packet to send
   * \param saddr The source Ipv4Address
   * \param daddr The destination Ipv4Address
   * \param sport The source port number
   * \param dport The destination port number
   */
  void Send (Ptr<Packet> packet,
             Ipv4Address saddr, Ipv4Address daddr, 
             uint16_t sport, uint16_t dport);
  void Send (Ptr<Packet> packet,
             Ipv4Address saddr, Ipv4Address daddr, 
             uint16_t sport, uint16_t dport, Ptr<Ipv4Route> route);
  /**
   * \brief Receive a packet up the protocol stack
   * \param p The Packet to dump the contents into
   * \param header IPv4 Header information
   * \param interface the interface from which the packet is coming.
   */
  // inherited from Ipv4L4Protocol
  virtual enum Ipv4L4Protocol::RxStatus Receive (Ptr<Packet> p,
                                                 Ipv4Header const &header,
                                                 Ptr<Ipv4Interface> interface);

  /**
   * \brief Receive an ICMP packet
   * \param icmpSource The IP address of the source of the packet.
   * \param icmpTtl The time to live from the IP header
   * \param icmpType The type of the message from the ICMP header
   * \param icmpCode The message code from the ICMP header
   * \param icmpInfo 32-bit integer carrying informational value of varying semantics.
   * \param payloadSource The IP source address from the IP header of the packet
   * \param payloadDestination The IP destination address from the IP header of the packet
   * \param payload Payload of the ICMP packet
   */
  virtual void ReceiveIcmp (Ipv4Address icmpSource, uint8_t icmpTtl,
                            uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo,
                            Ipv4Address payloadSource,Ipv4Address payloadDestination,
                            const uint8_t payload[8]);

  // From Ipv4L4Protocol
  virtual void SetDownTarget (Ipv4L4Protocol::DownTargetCallback cb);
  // From Ipv4L4Protocol
  virtual Ipv4L4Protocol::DownTargetCallback GetDownTarget (void) const;


  		protected:
		virtual void DoDispose (void);
  		virtual void NotifyNewAggregate ();
  		
  		private:
	  	Ptr<Node> m_node;
  		Ipv4EndPointDemux *m_endPoints;
  		BypassProtocol (const BypassProtocol &o);
  		BypassProtocol &operator = (const BypassProtocol &o);
  		std::vector<Ptr<BypassSocketImpl> > m_sockets;
  		Ipv4L4Protocol::DownTargetCallback m_downTarget;
  	};
}; //namespace ns3

#endif