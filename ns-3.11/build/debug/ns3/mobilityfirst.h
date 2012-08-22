/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
	//MobilityFirst packet processing 

#ifndef __MOBILITYFIRST_H__
#define __MOBILITYFIRST_H__

#include "mf_header.h"
#include "gstar_state.h"
#include "gstar_protocol.h"
#include "hop_protocol.h"
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
			uint8_t m_mac[6];
			
			int nNodes; //number of nodes
			int nFlows; //number of flows
			int mSeed; //seed for the RNG
			
			int isSource;
			int isDestination;
			
			int countFlows;
			
			std::string selectedSourceAddress;
			std::string selectedDestinationAddress;
			
			Ipv4Address m_mainAddress;
			/*Ipv4Address source_address_1;
			 Ipv4Address destination_address_1;
			 Ipv4Address source_address_2;
			 Ipv4Address destination_address_2;
			 Ipv4Address source_address_3;
			 Ipv4Address destination_address_3;*/
			
			std::vector<Ipv4Address> senders;
			//std::vector<Ipv4Address> receivers;
			
			//std::vector<Ipv4Address> mainAddresses;
			
			InterfaceTuple interfaces;
			
			GSTARProtocol gstar_pkt;
			HOPProtocol hop_pkt;  
			
			int LP_jitter_flag;
			int LSA_jitter_flag;
			int csyn_flag;
				//Ptr<Packet> csync_packet_retry;
			Timer m_CSYNC_Timer;
			void ReSendCSYNCTimer ();
			Ipv4Address m_interface;
			Ipv4Address rec_interface;
			
			std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
			
			Time m_LP_Interval; //time interval for re-sending link probes
			Timer m_LP_Timer;//timer function for sending link probe
			Timer m_LP_jitter_Timer;//timer function for delaying sending link probe
			
			Time m_LSA_Interval; //Time interval for re-sending LSAs
			Timer m_LSA_Timer; //Timer function for sending LSAs
			Timer m_LSA_jitter_Timer;//timer function for delaying sending LSA
			
			Time m_DLSA_Interval; //Time interval for re-sending LSAs
			Timer m_DLSA_Timer; //Timer function for sending LSAs
			
			Time m_Data_Interval; //time interval for sending data
			
			Timer m_Data_Timer;//timer function for resending data packets (if forward flag is 0)
			
			Time m_Update_Interval; //time interval for updating neighbor probability table
			Timer m_Update_Timer;//timer function for updating neighbor probability table
			
			Time m_thruput_Interval; //time interval for updating neighbor probability table
			Timer m_thruput_Timer;
			
			CSYNCSet csync_set;
			DataSavedSet data_saved_set;
			LSASavedSet lsa_set;
			
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
  			
  			void LPTimerExpire ();//function called when link probe timer expires to resend another link probe
			void LSATimerExpire ();//function called when LSA timer expires to resend another LSA
			void ProcessSummaryVectorACK(mf::MessageHeader summ_vec_pkt, Ipv4Address m_mainAddress, Ipv4Address my_iface, Ipv4Address neighbor_iface);//sends DLSA packets
			void DLSATimerExpire ();//callback funtion for sending DLSA
			void ReSendCSYN (Ipv4Address source);//callback funtion for resending CSYNC
			void ReSendData (Ipv4Address source);//callback funtion for resending data packets (if forward flag is 0)
			void SendData ();//function for sending Data
							 //void SendLinkProbeACK ();
			void UpdateTimerExpire ();//callback funtion for updating neighbor probability table
			void ReSendLSA (Ipv4Address source, Ipv4Address neighbor_addr, uint32_t seq_no);
			
			
				//checks if the packet was send by its own interface (would be used in#include "ns3/queue.h" LSA packets)
			bool IsMyOwnAddress (const Ipv4Address & a) const;
			
				// Find socket with local interface address iface
			Ptr<Socket> FindSocketWithAddress (const Ipv4Address iface) const;
			void selectNodes(void);
			std::string convertInt(int number);
			unsigned int Random(int min, int max);
			bool Find(std::vector<Ipv4Address>, Ipv4Address);
		};
	}
}


#endif
