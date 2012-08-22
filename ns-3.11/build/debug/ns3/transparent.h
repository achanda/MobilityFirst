/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
//MobilityFirst packet processing 

#ifndef __MOBILITYFIRST_H__
#define __MOBILITYFIRST_H__

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/event-garbage-collector.h"
#include "ns3/timer.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/queue.h"
#include "ns3/delay-jitter-estimation.h"

#include <vector>
#include <map>


namespace ns3 
{
	namespace mf 
	{
		class RoutingProtocol;
		class RoutingProtocol : public Ipv4RoutingProtocol
		{
		public:
  			//sets Parent, constructor and attributes (if required)
			//same as used in AODV and OLSR codes
			static TypeId GetTypeId (void);
			virtual TypeId GetInstanceTypeId (void) const;
			RoutingProtocol ();
  			virtual ~RoutingProtocol ();
			
		protected:	
			//creates socket for each interface
  			//as used in OLSR code
			virtual void DoStart (void);
			
		private:
			
			Ptr<Ipv4> m_ipv4;
			Ipv4Address m_mainAddress;
						
			std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
			
			//standard function used in both AODV and OLSR codes			
			virtual void NotifyInterfaceUp (uint32_t interface);
  			virtual void NotifyInterfaceDown (uint32_t interface);
  			virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  			virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  			virtual void SetIpv4 (Ptr<Ipv4> ipv4);
			virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;
			
  			void DoDispose ();
			
			void RecvMF (Ptr<Socket> socket);//receives packets
			
			// From Ipv4RoutingProtocol (used in both AODV and OLSR codes)
  			virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p,const Ipv4Header &header,Ptr<NetDevice> oif,Socket::SocketErrno &sockerr);
  			virtual bool RouteInput (Ptr<const Packet> p,const Ipv4Header &header,Ptr<const NetDevice> idev,UnicastForwardCallback ucb,MulticastForwardCallback mcb,LocalDeliverCallback lcb,ErrorCallback ecb);
  			
			//checks if the packet was send by its own interface (would be used in#include "ns3/queue.h" LSA packets)
			bool IsMyOwnAddress (const Ipv4Address & a) const;
			
			// Find socket with local interface address iface
			Ptr<Socket> FindSocketWithAddress (const Ipv4Address iface) const;
			
			
		};
	}
}


#endif

