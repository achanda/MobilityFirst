#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"
#include "ns3/object-vector.h"
#include "ns3/ipv4-route.h"

#include "bypass.h"
#include "bypass-header.h"
#include "bypass-socket-factory-impl.h"
#include "ipv4-end-point-demux.h"
#include "ipv4-end-point.h"
#include "ipv4-l3-protocol.h"
#include "bypass-socket-impl.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (BypassProtocol);
	TypeId BypassProtocol::GetTypeId (void)
	{
  		static TypeId tid = TypeId ("ns3::BypassProtocol").SetParent<Ipv4L4Protocol> ().AddConstructor<BypassProtocol> ());
  		return tid;
	}
	
	BypassProtocol::BypassProtocol () : m_endPoints (new Ipv4EndPointDemux ())
	{
	}

	BypassProtocol::~BypassProtocol ()
	{
	}
	
	void BypassProtocol::SetNode (Ptr<Node> node)
	{
  		m_node = node;
	}
	
	/*
 	* This method is called by AddAgregate and completes the aggregation
 	* by setting the node in the udp stack and link it to the ipv4 object
 	* present in the node along with the socket factory
 	*/
	void BypassProtocol::NotifyNewAggregate ()
	{
  		if (m_node == 0)
    	{
      		Ptr<Node> node = this->GetObject<Node> ();
      		if (node != 0)
        	{
          		Ptr<Ipv4> ipv4 = this->GetObject<Ipv4> ();
          		if (ipv4 != 0)
            	{
              		this->SetNode (node);
              		ipv4->Insert (this);
              		Ptr<BypassSocketFactoryImpl> udpFactory = CreateObject<BypassSocketFactoryImpl> ();
              		udpFactory->SetBypass (this);
              		node->AggregateObject (udpFactory);
              		this->SetDownTarget (MakeCallback (&Ipv4::Send, ipv4));
            	}
        	}
    	}
  	Object::NotifyNewAggregate ();
	}
	
	void BypassProtocol::DoDispose (void)
	{
  		for (std::vector<Ptr<BypassSocketFactoryImpl> >::iterator i = m_sockets.begin (); i != m_sockets.end (); i++)
    	{
      		*i = 0;
    	}
  		m_sockets.clear ();

  		if (m_endPoints != 0)
    	{
      		delete m_endPoints;
      		m_endPoints = 0;
    	}
  		m_node = 0;
  		m_downTarget.Nullify ();
  		Ipv4L4Protocol::DoDispose ();
	}
	
	Ptr<Socket> BypassProtocol::CreateSocket (void)
	{
  		//NS_LOG_FUNCTION_NOARGS ();
  		Ptr<BypassSocketImpl> socket = CreateObject<BypassSocketImpl> ();
  		socket->SetNode (m_node);
  		socket->SetUdp (this);
  		m_sockets.push_back (socket);
  		return socket;
	}
	
	Ipv4EndPoint *
BypassProtocol::Allocate (void)
{
  return m_endPoints->Allocate ();
}

Ipv4EndPoint *
BypassProtocol::Allocate (Ipv4Address address)
{
  //NS_LOG_FUNCTION (this << address);
  return m_endPoints->Allocate (address);
}

Ipv4EndPoint *
BypassProtocol::Allocate (uint16_t port)
{
  //NS_LOG_FUNCTION (this << port);
  return m_endPoints->Allocate (port);
}

Ipv4EndPoint *
BypassProtocol::Allocate (Ipv4Address address, uint16_t port)
{
  //NS_LOG_FUNCTION (this << address << port);
  return m_endPoints->Allocate (address, port);
}
Ipv4EndPoint *
BypassProtocol::Allocate (Ipv4Address localAddress, uint16_t localPort,
                         Ipv4Address peerAddress, uint16_t peerPort)
{
  //NS_LOG_FUNCTION (this << localAddress << localPort << peerAddress << peerPort);
  return m_endPoints->Allocate (localAddress, localPort,
                                peerAddress, peerPort);
}

void 
BypassProtocol::DeAllocate (Ipv4EndPoint *endPoint)
{
  //NS_LOG_FUNCTION (this << endPoint);
  m_endPoints->DeAllocate (endPoint);
}

void 
BypassProtocol::ReceiveIcmp (Ipv4Address icmpSource, uint8_t icmpTtl,
                            uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo,
                            Ipv4Address payloadSource,Ipv4Address payloadDestination,
                            const uint8_t payload[8])
{
  //NS_LOG_FUNCTION (this << icmpSource << icmpTtl << icmpType << icmpCode << icmpInfo 
  //                      << payloadSource << payloadDestination);
  uint16_t src, dst;
  src = payload[0] << 8;
  src |= payload[1];
  dst = payload[2] << 8;
  dst |= payload[3];

  Ipv4EndPoint *endPoint = m_endPoints->SimpleLookup (payloadSource, src, payloadDestination, dst);
  if (endPoint != 0)
    {
      endPoint->ForwardIcmp (icmpSource, icmpTtl, icmpType, icmpCode, icmpInfo);
    }
  else
    {
      //NS_LOG_DEBUG ("no endpoint found source=" << payloadSource <<
      //              ", destination="<<payloadDestination<<
      //              ", src=" << src << ", dst=" << dst);
    }
}


	
	enum Ipv4L4Protocol::RxStatus BypassProtocol::Receive (Ptr<Packet> packet,
                        Ipv4Header const &header,
                        Ptr<Ipv4Interface> interface)
    {
    	//TODO: implement bypass logic here
    	return Ipv4L4Protocol::RX_OK;
    }
    
    void BypassProtocol::Send (Ptr<Packet> packet, 
                     Ipv4Address saddr, Ipv4Address daddr,
                     uint16_t sport, uint16_t dport, Ptr<NetDevice> oif)
    {
    	//TODO: implement send logic
    }
    
    void BypassProtocol::SetDownTarget (Ipv4L4Protocol::DownTargetCallback callback)
	{
  		m_downTarget = callback;
	}

	Ipv4L4Protocol::DownTargetCallback BypassProtocol::GetDownTarget (void) const
	{
  		return m_downTarget;
	}
};