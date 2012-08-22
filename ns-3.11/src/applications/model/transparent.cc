/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//MobilityFirst packet processing 

#include "transparent.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-route.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv4-header.h"
#include "ns3/nstime.h"
#include "ns3/queue.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <vector>

#define GSTAR_PORT_NUMBER 9
using namespace std;

namespace ns3 
{
	
	namespace mf 
	{
        NS_LOG_COMPONENT_DEFINE ("BypassRoutingProtocol");
        NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);
		
		//standard function used in AODV and OLSR codes
		TypeId RoutingProtocol::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::mf::RoutingProtocol")
			.SetParent<Ipv4RoutingProtocol> ()
			.AddConstructor<RoutingProtocol> ();
			return tid;
		}
		
		TypeId RoutingProtocol::GetInstanceTypeId (void) const
		{
			return GetTypeId ();
		}
		
		RoutingProtocol::RoutingProtocol ()
		{
			LogComponentEnable("GstarRoutingProtocol",LOG_LEVEL_ALL);
			NS_LOG_DEBUG("Constructed gstar::RoutingProtocol");
			std::cout << "Constructed gstar::RoutingProtocol" << std::endl;
		}
		
		RoutingProtocol::~RoutingProtocol ()
		{
			NS_LOG_DEBUG("Destroyed gstar::RoutingProtocol");
		}
		
		//standard function used in AODV and OLSR codes
		void RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
		{
			NS_ASSERT (ipv4 != 0);
			NS_ASSERT (m_ipv4 == 0);
			NS_LOG_DEBUG("Starting gstar::RoutingProtocol::SetIpv4");
			
			m_ipv4 = ipv4;
		}
		
		//standard funtion used in AODV and OLSR codes
		void RoutingProtocol::DoDispose ()
		{
			m_ipv4 = 0;
			
			for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddresses.begin ();iter != m_socketAddresses.end (); iter++)
			{
				iter->first->Close ();
			}
			
			m_socketAddresses.clear ();
			
			Ipv4RoutingProtocol::DoDispose ();
		}
		
		void RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
		{
			
		}
		
		//creates sockets for each interface
		//exactly from OLSR code
		void RoutingProtocol::DoStart ()
		{
			//NS_LOG_DEBUG ("Starting GSTAR on node " << m_mainAddress);
			
			Ipv4Address loopback ("127.0.0.1");
			Ipv4Address addr;
			
			std::cout << m_ipv4 <<std::endl;
			for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
			{
				addr = m_ipv4->GetAddress (i, 0).GetLocal ();
				
				
				if (addr == loopback)
					continue;
				
				// Create a socket to listen only on this interface
				std::cout << "socket created at" << addr << std::endl;
				Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),UdpSocketFactory::GetTypeId ());
				socket->SetAllowBroadcast (true);
				InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), GSTAR_PORT_NUMBER);
				socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvMF,  this));
				if (socket->Bind (inetAddr))
				{
					NS_FATAL_ERROR ("Failed to bind() GSTAR socket");
				}
				socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
                
				m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
				
			}
			
			if( addr == "10.1.1.1")
			{
				Ptr<Packet> packet = Create<Packet> ();
				//m_txTrace (packet);
				//m_socket->Send (packet);
				//m_totBytes += m_pktSize;
				Ptr<Socket> socketn = FindSocketWithAddress ("10.1.1.1"); //gets the socket corresponding to the one on which link probe was rcvd
				NS_ASSERT (socketn);
				
				//sends packet to the above socket with receiver address set to the sender address of link probe
				socketn->SendTo (packet, 0, InetSocketAddress ("10.1.1.2", GSTAR_PORT_NUMBER));
				std::cout << "send pkt to 10.1.1.2" << std::endl;
			}	
			
			//NS_LOG_DEBUG ("GSTAR on node " << m_mainAddress << " started");
			
		}
		
		//receive and process packets
		//same as what OLSR code does
		void RoutingProtocol::RecvMF (Ptr<Socket> socket)
		{
			std::cout << "recv" << m_ipv4 << std::endl;
			//NS_LOG_FUNCTION(this);
			Ptr<Packet> receivedPacket;
			
			Address sourceAddress;
			receivedPacket = socket->RecvFrom (sourceAddress);
			//cout << "Pkt Size Rcvd" <<  receivedPacket->GetSize() << endl;
			
			InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
			Ipv4Address senderIfaceAddr = inetSourceAddr.GetIpv4 ();
			Ipv4Address receiverIfaceAddr = m_socketAddresses[socket].GetLocal ();
			std::cout << "Sender: "<< senderIfaceAddr << "Receiver: " << receiverIfaceAddr << std::endl;
			//NS_LOG_DEBUG ("OLSR node " << m_mainAddress << " received a OLSR packet from "<< senderIfaceAddr << " to " << receiverIfaceAddr);
			
			Ptr<Packet> packet = receivedPacket;
			
			if(receiverIfaceAddr == "10.1.1.2")
			{	
			Ptr<Socket> socketn = FindSocketWithAddress ("10.1.2.1"); //gets the socket corresponding to the one on which link probe was rcvd
			NS_ASSERT (socketn);
			
			//sends packet to the above socket with receiver address set to the sender address of link probe
			socketn->SendTo (packet, 0, InetSocketAddress ("10.1.2.2", GSTAR_PORT_NUMBER));
			std::cout << "send pkt to 10.1.2.2" << std::endl;
			}
			else 
			{
				std::cout << "finally rcvd pkt" << std::endl;	
			}
			std::cout << "out of recvMF" << std::endl;

		}
		
		//everything from here is same as in OLSR code
		Ptr<Ipv4Route> RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
		{
			NS_LOG_FUNCTION(this);
			
			Ptr<Ipv4Route> rtentry;
			return rtentry;
		}
		
		bool RoutingProtocol::RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,UnicastForwardCallback ucb,MulticastForwardCallback mcb,LocalDeliverCallback lcb, ErrorCallback ecb)
		{
			NS_LOG_FUNCTION(this);
			
			return false;
		}
		
		void RoutingProtocol::NotifyInterfaceUp (uint32_t i)
		{
			
		}
		
		void RoutingProtocol::NotifyInterfaceDown (uint32_t i)
		{
			
		}
		
		void RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
		{
			
		}
		
		void RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
		{
			
		}
		
		bool RoutingProtocol::IsMyOwnAddress (const Ipv4Address & a) const
		{
			// NS_LOG_FUNCTION(this);
			
			for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
			{
				Ipv4InterfaceAddress iface = j->second;
				
				if (a == iface.GetLocal ())
				{
					return true;
				}
			}
			return false;
		}
		
		// Find socket with local interface address iface
		Ptr<Socket> RoutingProtocol::FindSocketWithAddress (const Ipv4Address addr) const
		{   
			NS_LOG_FUNCTION(this);
			
			for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
			{
                Ptr<Socket> socket = j->first;
                Ipv4InterfaceAddress iface = j->second;
                if (iface.GetLocal() == addr)
					return socket;
			}
			
			Ptr<Socket> socket;
			return socket;
		}
		
	}
	
}