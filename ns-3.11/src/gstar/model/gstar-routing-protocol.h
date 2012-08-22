/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
//complete GSTAR protocol working

#ifndef __GSTAR_AGENT_IMPL_H__
#define __GSTAR_AGENT_IMPL_H__

#include "gstar_header.h"
#include "gstar_state.h"
#include "ns3/test.h"
#include "gstar_repositories.h"

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

#include <vector>
#include <map>


namespace ns3 
{
	namespace gstar 
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
                        EventGarbageCollector m_events; //used for scheduling NeighborTupleTimerExpire and LSATupleTimerExpire (erasing expired entries)
                        GSTARState m_state; //for accessing database manipulation (gstar_state.cc)
                        uint32_t LPSequenceNumber; //sequence number for link probe packets
                        uint32_t LSASequenceNumber; //sequence number for LSA packets
                        int LP_jitter_flag;
                        int LSA_jitter_flag;

			std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
                        Time m_LP_Interval; //time interval for re-sending link probes
			Time link_probe_history[10]; //saves time at which link probe was send
                        Timer m_LP_Timer;//timer function for sending link probe
                        Timer m_LP_jitter_Timer;//timer function for delaying sending link probe
                        
                        Time m_LSA_Interval; //Time interval for re-sending LSAs
                        Timer m_LSA_Timer; //Timer function for sending LSAs
                        Timer m_LSA_jitter_Timer;//timer function for delaying sending LSA
			
                        //standard function used in both AODV and OLSR codes			
                        virtual void NotifyInterfaceUp (uint32_t interface);
  			virtual void NotifyInterfaceDown (uint32_t interface);
  			virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  			virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  			virtual void SetIpv4 (Ptr<Ipv4> ipv4);
                        virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

  			void DoDispose ();
			
			void RecvGSTAR (Ptr<Socket> socket);//receives packets

			// From Ipv4RoutingProtocol (used in both AODV and OLSR codes)
  			virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p,const Ipv4Header &header,Ptr<NetDevice> oif,Socket::SocketErrno &sockerr);
  			virtual bool RouteInput (Ptr<const Packet> p,const Ipv4Header &header,Ptr<const NetDevice> idev,UnicastForwardCallback ucb,MulticastForwardCallback mcb,LocalDeliverCallback lcb,ErrorCallback ecb);
  			
  			void LPTimerExpire ();//function called when link probe timer expires to resend another link probe
                        void LSATimerExpire ();//function called when LSA timer expires to resend another LSA

			void SendLP ();//sends link probe
                        void SendLSA ();//sends LSA
			void SendLPACK (const Ipv4Address receiverIface,const Ipv4Address senderIface, uint32_t seq_no);//sends link probe ACK
			
                        //process link probe ACK and forms neighbor table
			void ProcessLPACK (Ipv4Address neighbor_add, uint32_t seq_no);
                       
                        //process LSA and forms LSA table
                        void ProcessLSA (gstar::MessageHeader lsaPacket,Ipv4Address addr, uint32_t seq_no);

                        void NeighborTupleTimerExpire (Ipv4Address neighbor_add); //function called when timer for a neighbor entry expires
                        void LSATupleTimerExpire (Ipv4Address addr); //function called when timer for a LSA entry expires

			//checks if the packet was send by its own interface (would be used in LSA packets)
			bool IsMyOwnAddress (const Ipv4Address & a) const;
                        
                        // Find socket with local interface address iface
                        Ptr<Socket> FindSocketWithAddress (const Ipv4Address iface) const;
		};
	}
}


#endif









