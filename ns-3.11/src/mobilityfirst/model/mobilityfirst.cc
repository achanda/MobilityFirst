/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

	//MobilityFirst packet processing 

#include "mobilityfirst.h"
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
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h" 
#include "ns3/yans-wifi-phy.h"
#include "ns3/queue.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <vector>
#include <sstream>

#define GSTAR_PORT_NUMBER 5000

	/// \brief Gets the delay between a given time and the current time.
	///
	/// If given time is previous to the current one, then this macro returns
	/// a number close to 0. This is used for scheduling events at a certain moment.
	///
#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
(time - Simulator::Now () + Seconds (0.000001)))
	//sends this packet on each interface with receiver address set to broadcast

#define CSYNC_INTERVAL_TIMER    Seconds (5)
#define DATA_INTERVAL_TIMER   Seconds (2)
#define LSA_INTERVAL_TIMER    Seconds (1)

using namespace std;

namespace ns3 
{
	
	namespace mf 
	{
        NS_LOG_COMPONENT_DEFINE ("GstarRoutingProtocol");
        NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);
		
			//standard function used in AODV and OLSR codes
		TypeId RoutingProtocol::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::mf::RoutingProtocol")
			.SetParent<Ipv4RoutingProtocol> ()
			.AddConstructor<RoutingProtocol> ()
			.AddAttribute ("HelloInterval", "HELLO messages emission interval.",
						   TimeValue (Seconds (1)),
						   MakeTimeAccessor (&RoutingProtocol::m_LP_Interval),
						   MakeTimeChecker ())
			.AddAttribute ("LSAInterval", "LSA messages emission interval.",
						   TimeValue (Seconds (2)),
						   MakeTimeAccessor (&RoutingProtocol::m_LSA_Interval),
						   MakeTimeChecker ())
			.AddAttribute ("DLSAInterval", "DLSA message creation interval.",
						   TimeValue (Seconds (10)),
						   MakeTimeAccessor (&RoutingProtocol::m_DLSA_Interval),
						   MakeTimeChecker ())
			.AddAttribute ("DataInterval", "Data messages start time.",
						   TimeValue (Seconds (40)),
						   MakeTimeAccessor (&RoutingProtocol::m_Data_Interval),
						   MakeTimeChecker ())
			.AddAttribute ("ThruputInterval", "Chunck creation time.",
						   TimeValue (Seconds (1)),
						   MakeTimeAccessor (&RoutingProtocol::m_thruput_Interval),
						   MakeTimeChecker ())
			.AddAttribute ("UpdateInterval", "Update interval.",
						   TimeValue (Seconds (1)),
						   MakeTimeAccessor (&RoutingProtocol::m_Update_Interval),
						   MakeTimeChecker ())
			.AddAttribute ("Nodes", "Number of nodes",
						   UintegerValue (6),
						   MakeUintegerAccessor (&RoutingProtocol::nNodes),
						   MakeUintegerChecker<uint32_t> ())
			.AddAttribute ("Flows", "Number of flows",
						   UintegerValue (1),
						   MakeUintegerAccessor (&RoutingProtocol::nFlows),
						   MakeUintegerChecker<uint32_t> ())
			.AddAttribute ("Seed", "Number of nodes",
						   UintegerValue (1),
						   MakeUintegerAccessor (&RoutingProtocol::mSeed),
						   MakeUintegerChecker<uint32_t> ());
			return tid;
		}
		
		TypeId RoutingProtocol::GetInstanceTypeId (void) const
		{
			return GetTypeId ();
		}
		
		RoutingProtocol::RoutingProtocol ()
		: m_LP_Timer (Timer::CANCEL_ON_DESTROY),  m_LP_jitter_Timer (Timer::CANCEL_ON_DESTROY),m_LSA_Timer (Timer::CANCEL_ON_DESTROY),m_LSA_jitter_Timer (Timer::CANCEL_ON_DESTROY), m_Data_Timer (Timer::CANCEL_ON_DESTROY)
		{
			LogComponentEnable("GstarRoutingProtocol",LOG_LEVEL_ALL);
			NS_LOG_DEBUG("Constructed gstar::RoutingProtocol");
			isSource = 0;
			isDestination = 0;
			countFlows = 0;
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
			
			m_LP_Timer.SetFunction (&RoutingProtocol::LPTimerExpire,this);//intilize link probe timer to go to LPTimer Expire function when scheduled
			m_LSA_Timer.SetFunction (&RoutingProtocol::LSATimerExpire, this);//intilize LSA timer to go to LSATimer Expire function when scheduled
			m_LP_jitter_Timer.SetFunction (&RoutingProtocol::LPTimerExpire, this);//adds a random jitter in sending a link probe
			m_LSA_jitter_Timer.SetFunction (&RoutingProtocol::LSATimerExpire, this);//adds a random jitter in sending a LSA
			m_Data_Timer.SetFunction (&RoutingProtocol::selectNodes,this);//initilizes data timer to the SendData function when scheduled
			m_Update_Timer.SetFunction (&RoutingProtocol::UpdateTimerExpire,this);//initilizes update timer to the UpdateTimerExpire function when scheduled
			m_DLSA_Timer.SetFunction (&RoutingProtocol::DLSATimerExpire, this);//intilize DLSA timer to go to DLSATimer Expire function when scheduled
			m_CSYNC_Timer.SetFunction (&RoutingProtocol::ReSendCSYNCTimer,this);
			 m_thruput_Timer.SetFunction (&RoutingProtocol::SendData,this);
			LP_jitter_flag = 0;
			LSA_jitter_flag = 0;
			csyn_flag = 0;
			/*source_address_1 = "10.1.1.1";
			destination_address_1 = "10.1.1.6";
			source_address_2 = "10.1.1.8";
			destination_address_2 = "10.1.1.9";
			source_address_3 = "10.1.1.7";
			destination_address_3 = "10.1.1.0";*/
			
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
		
		std::string RoutingProtocol::convertInt(int number)
		{
			std::stringstream ss;//create a stringstream
			ss << number;//add number to the stream
			return ss.str();//return a string with the contents of the stream
		}
		
		unsigned int RoutingProtocol::Random(int nLow, int nHigh)
		{
			return (rand() % (nHigh - nLow + 1)) + nLow;
		}
		
		bool RoutingProtocol::Find(std::vector<Ipv4Address> senders, Ipv4Address addr)
		{
			for(unsigned int i=0;i<senders.size();i++)
                		if(senders[i] == addr)
                        		return 1;
        		return 0;
		}
				
		void RoutingProtocol::selectNodes(void)
		{
				// TODO change this for simulations
			string addressPrefix = "10.1.1.";
			cout << "Selectnodes called on " << m_mainAddress << std::endl;
			countFlows++;
			
			srand(time(NULL));
			selectedSourceAddress = addressPrefix + convertInt(Random(1,nNodes-1)); //minus 1 to exclude the ferry node
			selectedDestinationAddress = addressPrefix + convertInt(Random(1,nNodes-1));
			
			while(Find(senders, Ipv4Address(selectedSourceAddress.c_str())))
			{
				 selectedSourceAddress = addressPrefix + convertInt(Random(1,nNodes-1));
			}

				//selectedSourceAddress = "10.1.1." + convertInt(selectn.GetInteger(1,nNodes));
				//selectedDestinationAddress = "10.1.1." + convertInt(selectn.GetInteger(1, nNodes));
			
				// make sure the two dudes are not the same dude
			while (selectedDestinationAddress == selectedSourceAddress)
			{
				//cout << Random(1,nNodes) << endl;
					//std::string selectedDestinationAddress = "10.1.1." + convertInt(selectn.GetInteger(1, nNodes));
				selectedDestinationAddress = "10.1.1." + convertInt(Random(1,nNodes-1));
				if (selectedDestinationAddress != selectedSourceAddress)
					break;
			}
			
				if (m_mainAddress == Ipv4Address(selectedSourceAddress.c_str()))
					SendData();
			
			cout << "##Source: " << selectedSourceAddress << " Flow: " << countFlows << std::endl;
			cout << "##Destination: " << selectedDestinationAddress << " Flow: " << countFlows << std::endl;
			
			senders.push_back(Ipv4Address(selectedSourceAddress.c_str()));
				//cout << "Time: " << Simulator::Now() << endl;
			
			if (countFlows < nFlows)
				m_Data_Timer.Schedule(m_Data_Interval);
			
		}
		
			//creates sockets for each interface
			//exactly from OLSR code
		void RoutingProtocol::DoStart ()
		{
			NS_LOG_DEBUG("DoStart");
			if (m_mainAddress == Ipv4Address ())
			{
                Ipv4Address loopback ("127.0.0.1");
                for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
                {
                        // Use primary address, if multiple
					Ipv4Address addr = m_ipv4->GetAddress (i, 0).GetLocal ();
					if (addr != loopback)
					{
						m_mainAddress = addr;
						break;
					}
                }
                NS_ASSERT (m_mainAddress != Ipv4Address ());
			}

			Ipv4Address sender;
			Ipv4Address receiver;
				
			
			NS_LOG_DEBUG ("Starting GSTAR on node " << m_mainAddress);
			
			Ipv4Address loopback ("127.0.0.1");
			bool canRungstar = false;
			
			for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
			{
				Ipv4Address addr = m_ipv4->GetAddress (i, 0).GetLocal ();
				
                if (addr == loopback)
					continue;
				
					// Create a socket to listen only on this interface
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
				
				canRungstar = true;
				
                Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (addr));
                Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
                if (wifi != 0)
                {
					Ptr<WifiMac> mac = wifi->GetMac ();
					if (mac != 0)
					{
						Mac48Address m_mac_addr = mac->GetAddress();
						m_mac_addr.CopyTo(m_mac);
						
						
						cout << m_mainAddress << " mac " << m_mac_addr << endl;
					}         
                }
			}
			
			if(canRungstar)
			{
                m_Data_Timer.Cancel();      		
                m_CSYNC_Timer.Cancel(); 
                
					//GSTAR
					LPTimerExpire ();
					LSATimerExpire ();
				
					//DTN
					//UpdateTimerExpire ();
					//m_DLSA_Timer.Schedule (Seconds (4));
				
                /*if (m_mainAddress == source_address_1)
				 {
				 //      cout << m_mainAddress << "in fromfile" << endl;                        
				 //     UniformVariable xlp (0.005,0.010);
				 m_Data_Timer.Schedule (m_Data_Interval) ;
				 //      m_Data_Timer.Schedule (m_Data_Interval + Seconds(xlp.GetValue()));
				 }
				 
				 if (m_mainAddress == source_address_2)
				 {
				 //cout << m_mainAddress << "in fromfile" << endl;                        
				 
				 m_Data_Timer.Schedule (m_Data_Interval);
				 }           
				 
				 if (m_mainAddress == source_address_3)
				 {
				 //cout << m_mainAddress << "in fromfile" << endl;                        
				 */
				
				 m_Data_Timer.Schedule (m_Data_Interval);
				 //}
                
			}
			NS_LOG_DEBUG ("GSTAR on node " << m_mainAddress << " started");
			
		}
		
			//receive and process packetsGetDataMode
			//same as what OLSR code does
		void RoutingProtocol::RecvMF (Ptr<Socket> socket)
		{
				//NS_LOG_FUNCTION(this);
			Ptr<Packet> receivedPacket;
			
			Address sourceAddress;
			receivedPacket = socket->RecvFrom (sourceAddress);
				//cout << "Pkt Size Rcvd" <<  receivedPacket->GetSize() << endl;
			
			InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
			Ipv4Address senderIfaceAddr = inetSourceAddr.GetIpv4 ();
			Ipv4Address receiverIfaceAddr = m_socketAddresses[socket].GetLocal ();
				//NS_LOG_DEBUG ("OLSR node " << m_mainAddress << " received a OLSR packet from "<< senderIfaceAddr << " to " << receiverIfaceAddr);
			
			Ptr<Packet> packet = receivedPacket;
			mf::MessageHeader gstarPacketHeader;
			packet->RemoveHeader (gstarPacketHeader);
			
			switch (gstarPacketHeader.GetMessageType ())
			{
				case 1: //received link probe message
                {
					Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (m_mainAddress));
					Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
						//if (wifi != 0) 
						//{                       
						int new_neighbor_flag = gstar_pkt.ProcessLP(gstarPacketHeader,wifi,receiverIfaceAddr, senderIfaceAddr, m_mainAddress);
                        
						if (new_neighbor_flag == 1)
						{
							Ptr<Packet> summ_vec_packet = gstar_pkt.SendSummaryVector(m_mainAddress);
							
							Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd
							NS_ASSERT (socket);
							
								//sends packet to the above socket with receiver address set to the sender address of link probe
							socket->SendTo (summ_vec_packet, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
						}
						//}
					/*else
					{
                        
							//Time rcvd = Simulator::Now ();
							//int64_t SETT_int = rcvd.GetNanoSeconds();
							// cout << " LP Pkt Size Rcvd" <<  receivedPacket->GetSize() << endl;
							//cout << m_mainAddress << "Time in RCVDLP" << SETT_int << endl;
						uint32_t LP_seq_no = gstarPacketHeader.GetSequenceNumber ();
							//cout << "node: " << m_mainAddress << "received LP seq no " << LP_seq_no << endl;
						
						Ptr<Packet> lpACK_packet = gstar_pkt.SendLPACK(receiverIfaceAddr, senderIfaceAddr, LP_seq_no, m_mainAddress);
						
						Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd
						NS_ASSERT (socket);
						
                	        //sends packet to the above socket with receiver address set to the sender address of link probe
						socket->SendTo (lpACK_packet, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
					}*/
					
					break;
                }
				case 2: //received link probe ACK message
                {
                        //cout << "RCVD LP ACK" << endl;                        
					Time rcvd = Simulator::Now ();
                        //cout << " LP ACK Pkt Size Rcvd" <<  receivedPacket->GetSize() << endl;
                        //int64_t SETT_int = rcvd.GetNanoSeconds();
                        //cout << m_mainAddress << "Time in RCVDLP ACK" << SETT_int << endl;
					
					uint32_t LP_ACK_seq_no = gstarPacketHeader.GetSequenceNumber ();
					Ipv4Address neighbor_addr = gstarPacketHeader.GetMessageAddr ();
					
                        //cout << "node: " << m_mainAddress << "received LP ACK seq no " << LP_ACK_seq_no << endl;
					
					int new_neighbor_flag = gstar_pkt.ProcessLPACK(neighbor_addr, LP_ACK_seq_no, m_mainAddress,receiverIfaceAddr, senderIfaceAddr);
					
					if (new_neighbor_flag == 1)
					{
						Ptr<Packet> summ_vec_packet = gstar_pkt.SendSummaryVector(m_mainAddress);
						
						Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd
						NS_ASSERT (socket);
						
                	        //sends packet to the above socket with receiver address set to the sender address of link probe
						socket->SendTo (summ_vec_packet, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
					}
					
					break;
                }
                case 3: //received LSA message
                {
					uint32_t LSA_rcvd_seq_no = gstarPacketHeader.GetSequenceNumber ();
					Ipv4Address LSA_addr = gstarPacketHeader.GetMessageAddr ();
					
					
					Ptr<Packet> lsa_ack_pkt = gstar_pkt.LSA_ACK(LSA_addr,LSA_rcvd_seq_no,m_mainAddress);
					Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd
  	                NS_ASSERT (socket);
					
						//sends packet to the above socket with receiver address set to the sender address of link probe
  	                socket->SendTo (lsa_ack_pkt, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
					
					
					int lsa_flag = 0;
                        //process this LSA omly if its not my LSA
					if (LSA_addr != m_mainAddress)
						lsa_flag = gstar_pkt.ProcessLSA(gstarPacketHeader,LSA_addr,LSA_rcvd_seq_no,m_mainAddress);
					
					if (lsa_flag == 1)
					{
						cout << "rebroadcasting" << endl;
							//Ptr<Packet> rebcast_lsa_pkt = gstar_pkt.GetPkt();
						
						Ptr<Packet> rebcast_lsa_pkt = Create<Packet> ();
						rebcast_lsa_pkt->AddHeader(gstarPacketHeader);
							//cout << "Pkt Size" <<   rebcast_lsa_pkt->GetSize() << endl;
						/*for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
						 {
						 Ipv4Address bcast = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
						 i->first->SendTo (rebcast_lsa_pkt, 0, InetSocketAddress (bcast, GSTAR_PORT_NUMBER));
						 }*/
						
						NeighborSet curr_neighbor_list = gstar_pkt.GetNeighborTable();
						
						for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
						{
								//cout << "sending to " << it->neighbor_iface << endl;
							Ptr<Socket> socket = FindSocketWithAddress (it->my_ifac); //gets the socket corresponding to the one on which link probe was rcvd/
							NS_ASSERT (socket);
							
								//sends packet to the above socket with receiver address set to the sender address of link probe
							
							socket->SendTo (rebcast_lsa_pkt, 0, InetSocketAddress (it->neighbor_iface, GSTAR_PORT_NUMBER));
							
							LSATupleSaved lsa_tuple;
							
							lsa_tuple.source = LSA_addr;
							lsa_tuple.destination = it->neighbor_iface;
							lsa_tuple.seq_no = LSA_rcvd_seq_no;
							lsa_tuple.lsa_pkt = rebcast_lsa_pkt;
							
							Time now = Simulator::Now ();    //current time                                 
							
							EventId id = Simulator::Schedule (DELAY (now + LSA_INTERVAL_TIMER),
															  &RoutingProtocol::ReSendLSA,
															  this,LSA_addr,it->neighbor_iface,LSA_rcvd_seq_no);
							
							lsa_tuple.id = id; 
							lsa_tuple.expiration_time = now + LSA_INTERVAL_TIMER;
							
							lsa_set.push_back (lsa_tuple);
							
							
						}
					}
					
					break;
                }
                case 4: //recieved summary vector10from10.1.1.2
					
                {
                        //cout << m_mainAddress << " rcvd summ vect" << endl;
					uint32_t summ_vec_seq_no = gstarPacketHeader.GetSequenceNumber ();
					Ptr<Packet> summ_vec_ACK_pkt = gstar_pkt.SendSummaryVectorACK(gstarPacketHeader,summ_vec_seq_no,m_mainAddress);
					
					Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd
  	                NS_ASSERT (socket);
					
						//sends packet to the above socket with receiver address set to the sender address of link probe
  	                socket->SendTo (summ_vec_ACK_pkt, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
					break;
                }
					
                case 5: //recieved summary vector ACK
                {
						//cout << "node: " << m_mainAddress << "received summ vec ACK "  << endl;
					ProcessSummaryVectorACK(gstarPacketHeader,m_mainAddress,receiverIfaceAddr,senderIfaceAddr);
					
					break;
                }
					
                case 6: //received DLSA message CSYNCTuple csyn = hop_pkt.GetCSYNC();
					
                {
                        //cout << m_mainAddress << "rcvd DLSA" << endl;
					uint32_t DLSA_rcvd_seq_no = gstarPacketHeader.GetSequenceNumber ();                        
					Ipv4Address DLSA_addr = gstarPacketHeader.GetMessageAddr ();
					int dlsa_flag;
                        //process this DLSA only if its not sent by myself
					if (DLSA_addr != m_mainAddress)
						dlsa_flag = gstar_pkt.ProcessDLSA(gstarPacketHeader,DLSA_addr,DLSA_rcvd_seq_no,m_mainAddress);
                        //cout << "2" << endl;
					if (dlsa_flag == 1)
					{
						Ptr<Packet> summ_vec_packet = gstar_pkt.SendSummaryVector(m_mainAddress);
						
						NeighborSet curr_neighbor_list = gstar_pkt.GetNeighborTable();
						
						for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
						{
								//cout << "sending to " << it->neighbor_iface << endl;
							Ptr<Socket> socket = FindSocketWithAddress (it->my_ifac); //gets the socket corresponding to the one on which link probe was rcvd/
							NS_ASSERT (socket);
							
								//sends packet to the above socket with receiver address set to the sender address of link probe
							socket->SendTo (summ_vec_packet, 0, InetSocketAddress (it->neighbor_iface, GSTAR_PORT_NUMBER));
							
						}
					}
					
					break;
                }
					
                case 7: //received CSYNC message
                {
					cout << "csync rcvd" << endl;
					cout << m_mainAddress;
					
					CSYNCTuple csyn = hop_pkt.SendCSYNCACK(gstarPacketHeader,m_mainAddress);
					
					Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd
  	                NS_ASSERT (socket);
					cout << "sending CSYNC ACK" << endl;
						//sends packet to the above socket with receiver address set to the sender address of link probe
  	                socket->SendTo (csyn.csync_ack_pkt, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
					cout << receiverIfaceAddr << "My inte" << senderIfaceAddr << "Neigh inte" << endl;
                        //CSYNCTuple csyn = hop_pkt.GetCSYNC();
					if (csyn.flag == 1)
					{
						cout << "sending to " << csyn.destination << endl;
						interfaces = gstar_pkt.GetInterfaces (csyn.destination); 
						
						
						if (interfaces.my_interface != "0.0.0.0")
						{
							Ptr<Socket> socket = FindSocketWithAddress (interfaces.my_interface);
							NS_ASSERT (socket);
							
								//sends packet to the above socket with receiver address set to the sender address of link probe
							socket->SendTo (csyn.csync_pkt, 0, InetSocketAddress (interfaces.neighbor_interface, GSTAR_PORT_NUMBER));
						}
						
						Time now = Simulator::Now ();    //current time                                 
                        
						EventId id = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
														  &RoutingProtocol::ReSendCSYN,
														  this,csyn.source);
						
						csyn.id = id; 
						csyn.csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
						
						csync_set.push_back (csyn);
							//cout << "csync send" << endl;
						
							////csync_packet_retry = csyn.csync_pkt;
							////m_interface = interfaces.my_interface;
							////rec_interface = interfaces.neighbor_interface;
							////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);
					}
					
					break;
                }
					
                case 8: //received CSYNC ACK message
                {
					cout << "csyn ack rcvd" << endl;
					cout << m_mainAddress << "from" << senderIfaceAddr;
					
                        ////m_CSYNC_Timer.Cancel(); 
					
					const mf::MessageHeader::CSYNC_ACK &csync_ack_c = gstarPacketHeader.GetCSYNC_ACK();
					
					for (CSYNCSet::iterator it = csync_set.begin ();it != csync_set.end (); it++)
					{
						if (it->source == csync_ack_c.src)
						{
							Simulator::Cancel(it->id);
							it = csync_set.erase (it);
							break;
						}
					}
					
                        //std::vector< Ptr<Packet> > test = hop_pkt.ProcessCSYNCACK(gstarPacketHeader, m_mainAddress);
					DataTupleSaved saved_data;
					
					saved_data.data_pkts = hop_pkt.ProcessCSYNCACK(gstarPacketHeader, m_mainAddress);
					
                        //int size = test.size ();
					int size = saved_data.data_pkts.size ();
					cout << "Size in MF" << size << endl;
                        int flag = gstar_pkt.GetForwardFlag(csync_ack_c.dest); 
					//int flag = 1;
                        //cout << "First Time: " << flag << endl;
					if (flag == 1 || flag == 2)
					{
							//cout << size << endl;
						for ( int i=0; i<(size); i++)
						{                        
								//Ptr<Packet> data_pkt = test[i];
							Ptr<Packet> data_pkt = saved_data.data_pkts[i];
							Ptr<Socket> socket = FindSocketWithAddress (receiverIfaceAddr); //gets the socket corresponding to the one on which link probe was rcvd 
																							//cout << "Pkt Size" <<  data_pkt->GetSize() << endl;
							cout << "sending data pkts" << endl;
							NS_ASSERT (socket);
							
								//sends packet to the above socket with receiver address set to the sender address of link probe
							socket->SendTo (data_pkt, 0, InetSocketAddress (senderIfaceAddr, GSTAR_PORT_NUMBER));
						}
					}
					if (flag == 0 && size != 0)
					{
						cout << m_mainAddress << " storing" << endl;
						saved_data.source = csync_ack_c.src;
						saved_data.destination = csync_ack_c.dest;
						
						Time now = Simulator::Now ();    //current time      
						
						EventId id_cc = Simulator::Schedule (DELAY (now + DATA_INTERVAL_TIMER),
															 &RoutingProtocol::ReSendData,
															 this,saved_data.source);
						
						saved_data.id = id_cc;
						saved_data.expiration_time = now;
						
						data_saved_set.push_back (saved_data);
					}
					
					if (size != 0 && flag != 0)
					{
							////csync_packet_retry = saved_data.data_pkts[size-1];
							////m_interface = receiverIfaceAddr;
							////rec_interface = senderIfaceAddr;
							////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);
						CSYNCTuple csync;
						mf::MessageHeader pkt_header;
	                        //test[0]->RemoveHeader (pkt_header);
						saved_data.data_pkts[size-1]->RemoveHeader (pkt_header);
							//csync.csync_pkt = test[size-1];
							//cout << "1" << endl;
							////csync.csync_pkt = saved_data.data_pkts[size-1];
						const mf::MessageHeader::CSYNC &csyn = pkt_header.GetCSYNC ();
						csync.source = csyn.src;
						csync.destination = csyn.dest;
						
							//cout << "2" << endl;
						
						Ptr<Packet> csync_packet_retry =  Create<Packet> ();
						csync_packet_retry->AddHeader(pkt_header);
							//cout << "3" << endl;
						csync.csync_pkt = csync_packet_retry;
						
						Time now = Simulator::Now ();    //current time      
						
						EventId id_c = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
															&RoutingProtocol::ReSendCSYN,
															this,csync.source);
						
						csync.id = id_c; 
						csync.csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
						
						csync_set.push_back (csync);
							////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);
					}
					break;
                }
					
                case 9: //received DATA message
                {
                        //cout << m_mainAddress;
					cout << "@@@Received data packet at " << m_mainAddress << " from" << senderIfaceAddr << endl;
					hop_pkt.RcvdDataPkt(gstarPacketHeader,m_mainAddress);
					
					
                }
					
                case 10:
                {
					
					uint32_t seq_no = gstarPacketHeader.GetSequenceNumber ();
					Ipv4Address neighbor_addr = gstarPacketHeader.GetMessageAddr ();
					
					mf::MessageHeader::LSA_ACK &lsa_ack = gstarPacketHeader.GetLSA_ACK ();
					
					for (LSASavedSet::iterator it = lsa_set.begin ();it != lsa_set.end (); it++)
					{
						if (it->source == lsa_ack.src && it->destination == neighbor_addr && it->seq_no == seq_no)
						{
							Simulator::Cancel(it->id);
							it = lsa_set.erase (it);
							break;
						}
					}
                }
					
			}
		}
		
			//callback funtion for sending link probe
		void RoutingProtocol::LPTimerExpire ()
		{
				//NS_LOG_FUNCTION(this);
				//cout << "i is " << i << endl;
			Ptr<Packet> lp_packet;
				//Time rcvd = Simulator::Now ();
				//int64_t SETT_int = rcvd.GetNanoSeconds();
			
				//cout << m_mainAddress << "SendLP" << endl;const mf::MessageHeader::LSA &lsa = lsaPacket.GetLSA ();
			if (LP_jitter_flag == 0)
			{
                LP_jitter_flag = 1;
					//cout << LP_jitter_flag << endl;
                UniformVariable xlp (0,0.001);
                m_LP_jitter_Timer.Schedule (Seconds(xlp.GetValue()));// delays the sending of link probe
			}
			
			else
			{
				lp_packet = gstar_pkt.SendLP (m_mainAddress, m_mac);
                
                
                
					//sends this packet on each interface with receiver address set to broadcast
				for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    	        {
						//cout << "sending" << endl;
						//Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (i->second.GetLocal ()));
						//Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
						//if (wifi != 0)
						//{
						//Ptr<WifiRemoteStationManager> = wifi->GetRemoteStationManager();
					
					
						//Ptr<WifiPhy> phy = wifi->GetPhy ();   
						//double channel_freq = phy->GetChannelFrequencyMhz();
						//uint16_t channel_no = phy->GetChannelNumber();
						//cout << m_mainAddress << "Channel Number: " << channel_no << endl;
						//uint32_t mode_nos = phy->GetNModes ();
						//cout << m_mainAddress << "Modes: " << mode_nos << endl;
						//uint32_t rate = mode.GetPhyRate();
						//cout << m_mainAddress << "Rate: " << rate << endl;
					
						//} 
					Ipv4Address bcast = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
						//cout << "2" << endl;
					i->first->SendTo (lp_packet, 0, InetSocketAddress (bcast, GSTAR_PORT_NUMBER));
						//cout << "LP Send" << endl;
    	        }
				
                LP_jitter_flag = 0;
                m_LP_Timer.Schedule (m_LP_Interval);//reschedule timer
													//cout << m_mainAddress << endl;
													//cout << "In sendLP" << endl;
			}
		}
		
			//callback funtion for sending LSA
		void RoutingProtocol::LSATimerExpire ()
		{
				//NS_LOG_FUNCTION(this);
			
			Ptr<Packet> lsa_packet;
			
			/*if (LSA_jitter_flag == 0)
			 {
			 LSA_jitter_flag = 1;
			 //cout << LP_jitter_flag << endl;
			 UniformVariable xlp (0,0.001);
			 m_LSA_jitter_Timer.Schedule (Seconds(xlp.GetValue()));// delays the sending of link probe
			 }
			 
			 else
			 {*/
	        lsa_packet = gstar_pkt.SendLSA (m_mainAddress);
			uint32_t LSA_seq_no = gstar_pkt.GetLSASeqNo ();
			cout << m_mainAddress << " in LSA LSA seq no: " << LSA_seq_no << endl;
			
                //sends this packet on each interface with receiver address set to broadcast
			/*for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)    	          {
			 Ipv4Address bcast = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
			 i->first->SendTo (lsa_packet, 0, InetSocketAddress (bcast, GSTAR_PORT_NUMBER));
			 }*/
			NeighborSet curr_neighbor_list = gstar_pkt.GetNeighborTable();
			
			for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
			{
					//cout << "sending to " << it->neighbor_iface << endl;
				Ptr<Socket> socket = FindSocketWithAddress (m_mainAddress); //gets the socket corresponding to the one on which link probe was rcvd/
				NS_ASSERT (socket);
                
					//sends packet to the above socket with receiver address set to the sender address of link probe
				socket->SendTo (lsa_packet, 0, InetSocketAddress (it->neighbor_iface, GSTAR_PORT_NUMBER));
				
				LSATupleSaved lsa_tuple;
				
				lsa_tuple.source = m_mainAddress;
				lsa_tuple.destination = it->neighbor_iface;
				lsa_tuple.seq_no = LSA_seq_no;
				lsa_tuple.lsa_pkt = lsa_packet;
				
				Time now = Simulator::Now ();    //current time                                 
				
				EventId id = Simulator::Schedule (DELAY (now + LSA_INTERVAL_TIMER),
												  &RoutingProtocol::ReSendLSA,
												  this,m_mainAddress,it->neighbor_iface,LSA_seq_no);
				
				lsa_tuple.id = id; 
				lsa_tuple.expiration_time = now + LSA_INTERVAL_TIMER;
				
				lsa_set.push_back (lsa_tuple);
				
				cout << "sending" << endl;
				
			}
			
				// LSA_jitter_flag = 0;
			m_LSA_Timer.Schedule (m_LSA_Interval);//reschedule timer
												  //}
		}
		
			//sends DLSA packets
		void RoutingProtocol::ProcessSummaryVectorACK(mf::MessageHeader summ_vec_pkt, Ipv4Address m_mainAddress, Ipv4Address my_iface, Ipv4Address neighbor_iface)
		{
			const mf::MessageHeader::summary_vector &summ_vec = summ_vec_pkt.Get_summary_vector ();
			
			Ipv4Address *addr_array;
			int j;
			
			addr_array = (Ipv4Address*)malloc(summ_vec.length*sizeof(Ipv4Address));
			
			j=0;
				// gets the node addresses in summary vector ACK
			for (std::vector<Ipv4Address>::const_iterator i = summ_vec.nodeAddresses.begin (); i != summ_vec.nodeAddresses.end (); i++)
			{
                addr_array[j] = *i;
                j++;
			}
			
			for(int k=0; k<j; k++)
			{
					//cout << m_mainAddress << "Addr" << addr_array[k] << endl;
                Ptr<Packet> DLSA_pkt = gstar_pkt.SendDLSA(addr_array[k],m_mainAddress);
                
                Ptr<Socket> socket = FindSocketWithAddress (my_iface); //gets the socket corresponding to the one on which link probe was rcvd
				NS_ASSERT (socket);
                
					//sends packet to the above socket with receiver address set to the sender address of link probe
                socket->SendTo (DLSA_pkt, 0, InetSocketAddress (neighbor_iface, GSTAR_PORT_NUMBER));
                
			}
		}
		
			//callback funtion for sending DLSA
		void RoutingProtocol::DLSATimerExpire ()
		{
				//cout << m_mainAddress << ": Creating own DLSA" << endl;
			Ptr<Packet> DLSA_pkt = gstar_pkt.SendMyDLSA(m_mainAddress);
			
			mf::MessageHeader gstarPacketHeader;
			DLSA_pkt->RemoveHeader (gstarPacketHeader);
			
			uint32_t DLSA_rcvd_seq_no = gstarPacketHeader.GetSequenceNumber ();                        
			Ipv4Address DLSA_addr = gstarPacketHeader.GetMessageAddr ();
			int dlsa_flag;
			dlsa_flag = gstar_pkt.ProcessDLSA(gstarPacketHeader,DLSA_addr,DLSA_rcvd_seq_no,m_mainAddress);
			
				//cout << "DLSA Flag" << dlsa_flag << endl;
			Ptr<Packet> summ_vec_packet = gstar_pkt.SendSummaryVector(m_mainAddress);
			
			NeighborSet curr_neighbor_list = gstar_pkt.GetNeighborTable();
			
			for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
			{
					//cout << "sending to " << it->neighbor_iface << endl;
                Ptr<Socket> socket = FindSocketWithAddress (it->my_ifac); //gets the socket corresponding to the one on which link probe was rcvd
				NS_ASSERT (socket);
                
					//sends packet to the above socket with receiver address set to the sender address of link probe
				socket->SendTo (summ_vec_packet, 0, InetSocketAddress (it->neighbor_iface, GSTAR_PORT_NUMBER));
			}
			m_DLSA_Timer.Schedule (m_DLSA_Interval);//reschedule timer
		}
		
		void RoutingProtocol::ReSendCSYNCTimer ()
		{
			Ptr<Socket> socket = FindSocketWithAddress ( m_interface); //gets the socket corresponding to the one on which link probe was rcvd
			NS_ASSERT (socket);
			
				//sends packet to the above socket with receiver address set to the sender address of link probe
				////socket->SendTo (csync_packet_retry, 0, InetSocketAddress ( rec_interface, GSTAR_PORT_NUMBER));
			
				////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);
			
		}
		
			//callback funtion for resending CSYNC
		void RoutingProtocol::ReSendCSYN (Ipv4Address source)
		{
				//cout << m_mainAddress << endl;              
			
			cout << m_mainAddress << "in resend csyn" << endl;
			
			for (CSYNCSet::iterator it = csync_set.begin ();it != csync_set.end (); it++)
			{
                if (it->source == source)
                {
					InterfaceTuple interface = gstar_pkt.GetInterfaces (it->destination);
                        //cout << interface.my_interface << endl;
                        //cout << it->destination << endl;
					if (it->csync_expiration_time < Simulator::Now ())
					{
						if (interface.my_interface != "0.0.0.0")       
						{
							Ptr<Socket> socket = FindSocketWithAddress (interface.my_interface); //gets the socket corresponding to the one on which link probe was rcvd
							NS_ASSERT (socket);
							
								//sends packet to the above socket with receiver address set to the sender address of link probe
							socket->SendTo (it->csync_pkt, 0, InetSocketAddress (interface.neighbor_interface, GSTAR_PORT_NUMBER));
							
							Time now = Simulator::Now ();    //current time    
							
							EventId id_r = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
																&RoutingProtocol::ReSendCSYN,
																this,it->source);
							
							it->id = id_r;
							it->csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
						}
						else
						{
							Time now = Simulator::Now ();    //current time      
							
							EventId id_c = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
																&RoutingProtocol::ReSendCSYN,
																this,it->source);
							
							it->csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
							it->id = id_c;
							
						}
					}
					else
					{
						EventId id_r = Simulator::Schedule (DELAY (it->csync_expiration_time),
															&RoutingProtocol::ReSendCSYN,
															this,it->source);
						
						it->id = id_r;       
					}
					
					break;
                }
			}
		}
		
			//function for sending Data
		void RoutingProtocol::SendData ()
		{
			cout << "#####" << "SendData at node " << m_mainAddress << "#####" << endl;
			Ptr<Packet> test; 
			InterfaceTuple interface_s;
			CSYNCTuple csync;

					test = hop_pkt.FromFile(m_mainAddress, Ipv4Address(selectedSourceAddress.c_str()), Ipv4Address(selectedDestinationAddress.c_str()));
					interface_s = gstar_pkt.GetInterfaces (Ipv4Address(selectedDestinationAddress.c_str())); 
					cout << interface_s.my_interface << endl;
					cout << interface_s.neighbor_interface << endl;
					csync.csync_pkt = test;
					csync.source = Ipv4Address(selectedSourceAddress.c_str());
					csync.destination = Ipv4Address(selectedDestinationAddress.c_str());
			
					
					/*Time now = Simulator::Now ();    //current time      
			
					EventId id_c = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
												&RoutingProtocol::ReSendCSYN,
												this,csync.source);
			
					csync.id = id_c; 
					csync.csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
			
					csync_set.push_back (csync);*/
				////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);
			
			
			/*
			if (m_mainAddress == source_address_1)
			{
                test = hop_pkt.FromFile(m_mainAddress, source_address_1, destination_address_1);
                cout << "in send data" << endl;
                interface_s = gstar_pkt.GetInterfaces (destination_address_1); 
                cout << interface_s.my_interface << endl;
                cout << interface_s.neighbor_interface << endl;
                csync.csync_pkt = test;
                csync.source = source_address_1;
                csync.destination = destination_address_1;
					//csync.csync_ack_pkt = test;
			}
			
			if (m_mainAddress == source_address_2)
			{ 
                test = hop_pkt.FromFile(m_mainAddress, source_address_2, destination_address_2);
                cout << "in send data" << endl;
                interface_s = gstar_pkt.GetInterfaces (destination_address_2);
                csync.csync_pkt = test;
                csync.source = source_address_2;
                csync.destination = destination_address_2;
					//csync.csync_ack_pkt = test;
			}
			
			if (m_mainAddress == source_address_3)
			{ 
                test = hop_pkt.FromFile(m_mainAddress, source_address_3, destination_address_3);
                interface_s = gstar_pkt.GetInterfaces (destination_address_3);
                csync.csync_pkt = test;
                csync.source = source_address_3;
                csync.destination = destination_address_3;
					//csync.csync_ack_pkt = test;
			}
			
				//EventId id_c = Simulator::Schedule (m_CSYNC_Interval,
				//              &RoutingProtocol::ReSendCSYN,
				//              this,csync.source);
				//csync.id = id_c; 
				//csync_set.push_back (csync);
			 
			 */
			if(test != NULL) 
			{
				Time now = Simulator::Now ();    //current time      
				
				EventId id_c = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
													&RoutingProtocol::ReSendCSYN,
													this,csync.source);
				
				csync.id = id_c; 
				csync.csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
				
				csync_set.push_back (csync);
			if (interface_s.my_interface != "0.0.0.0")
			{
				
                cout << "sending CSYNC" << endl;
                Ptr<Socket> socket = FindSocketWithAddress (interface_s.my_interface); //gets the socket corresponding to the one on which link probe was rcvd
																					   //Ptr<Socket> socket = FindSocketWithAddress ("10.1.1.1"); 
       	        NS_ASSERT (socket);
                
					//sends packet to the above socket with receiver address set to the sender address of link probe
                socket->SendTo (test, 0, InetSocketAddress (interface_s.neighbor_interface, GSTAR_PORT_NUMBER));
					//socket->SendTo (test, 0, InetSocketAddress ("10.1.1.2", GSTAR_PORT_NUMBER));
			}
			}
			
			m_thruput_Timer.Schedule (m_thruput_Interval);
			
				//interfaces.my_interface = "10.1.1.1";
				//interfaces.neighbor_interface = "10.1.2.2";
				////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);        
		}
		
			//callback funtion for resending data packets (if forward flag is 0)
		void RoutingProtocol::ReSendData (Ipv4Address source)
		{
			for (DataSavedSet::iterator it = data_saved_set.begin ();it != data_saved_set.end (); it++)
			{
                if (it->source == source)
                {
					if(it->expiration_time < Simulator::Now ())
					{
						int flag = gstar_pkt.GetForwardFlag(it->destination); 
							//int flag = 1;
						cout << "Resending " << flag << endl;
						Ptr<Packet> data_pkt;
						if (flag == 1 || flag == 2)
						{
								//cout << size << endl;
							InterfaceTuple interface = gstar_pkt.GetInterfaces (it->destination);
							
							if(interface.my_interface != "0.0.0.0")
							{
								int size = it->data_pkts.size();
								for ( int i=0; i<size; i++)
								{                        
									data_pkt = it->data_pkts[i];
									Ptr<Socket> socket = FindSocketWithAddress (interface.my_interface); //gets the socket corresponding to the one on which link probe was rcvd     
																										 //cout << "Pkt Size" <<  data_pkt->GetSize() << endl;
																										 //cout << "sending data pkts" << endl;
									NS_ASSERT (socket);
									
										//sends packet to the above socket with receiver address set to the sender address of link probe
									socket->SendTo (data_pkt, 0, InetSocketAddress (interface.neighbor_interface, GSTAR_PORT_NUMBER));
								}
                                
								
								if (it->data_pkts.size() != 0)
								{
									CSYNCTuple csync;
									
									csync.csync_pkt = data_pkt;
										//   const mf::MessageHeader::DATA &data = pkt_header.GetDATA ();
									csync.source = it->source;
									csync.destination = it->destination;
									
									Time now = Simulator::Now ();    //current time    
									
									EventId id_r = Simulator::Schedule (DELAY (now + CSYNC_INTERVAL_TIMER),
																		&RoutingProtocol::ReSendCSYN,
																		this,it->source);
									
									csync.id = id_r;
									csync.csync_expiration_time = now + CSYNC_INTERVAL_TIMER;
									
									csync_set.push_back (csync);
										////m_CSYNC_Timer.Schedule (m_CSYNC_Interval);
								}
								
								Simulator::Cancel(it->id);
								it = data_saved_set.erase (it);
							}
							else
							{
								cout << m_mainAddress << " again storing" << endl;
								Time now = Simulator::Now ();    //current time   
								EventId id_cc = Simulator::Schedule (DELAY (now + DATA_INTERVAL_TIMER),
																	 &RoutingProtocol::ReSendData,
																	 this,it->source);
                                
								it->id = id_cc;
							}
							
						}
						else
						{
							cout << m_mainAddress << " again storing" << endl;
							Time now = Simulator::Now ();    //current time   
							EventId id_cc = Simulator::Schedule (DELAY (now + DATA_INTERVAL_TIMER),
																 &RoutingProtocol::ReSendData,
																 this,it->source);
							
							it->id = id_cc;
						}
						
					}
					else
					{
						EventId id_r = Simulator::Schedule (DELAY (it->expiration_time),
															&RoutingProtocol::ReSendData,
															this,it->source);
						it->id = id_r;
					}
					
					break;
                }
			}
		}
		
			//callback funtion for updating neighbor probability table
		void RoutingProtocol::UpdateTimerExpire ()
		{
			gstar_pkt.UpdateNeighborProbTable (m_mainAddress);
			m_Update_Timer.Schedule (m_Update_Interval);
		}
		
		void RoutingProtocol::ReSendLSA (Ipv4Address source, Ipv4Address neighbor_addr, uint32_t seq_no)
		{
				//cout << m_mainAddress << endl;              
			
			cout << m_mainAddress << "in resend lsa for " << seq_no << endl;
			
			for (LSASavedSet::iterator it = lsa_set.begin ();it != lsa_set.end (); it++)
			{
                if (it->source == source && it->destination == neighbor_addr && it->seq_no == seq_no)
                {
					if (it->expiration_time < Simulator::Now ())
					{
						Ptr<Socket> socket = FindSocketWithAddress (m_mainAddress); //gets the socket corresponding to the one on which link probe was rcvd/
						NS_ASSERT (socket);
						
							//sends packet to the above socket with receiver address set to the sender address of link probe
						socket->SendTo (it->lsa_pkt, 0, InetSocketAddress (it->destination, GSTAR_PORT_NUMBER));
						
						Time now = Simulator::Now ();    //current time    
						
						EventId id_r = Simulator::Schedule (DELAY (now + LSA_INTERVAL_TIMER),
															&RoutingProtocol::ReSendLSA,
															this,it->source,it->destination,it->seq_no);
                        
						it->id = id_r;
						it->expiration_time = now + LSA_INTERVAL_TIMER;
						
					}
					else
					{
						EventId id_r = Simulator::Schedule (DELAY (it->expiration_time),
															&RoutingProtocol::ReSendLSA,
															this,it->source,it->destination,it->seq_no);
						it->id = id_r;
					}
					
					break;
                }
			}
		}
		
			//everything from here is same as in OLSR code
		Ptr<Ipv4Route> RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
		{
			NS_LOG_FUNCTION(this);
			
			Ptr<Ipv4Route> rtentry;
			return rtentry;
		}
		
		bool RoutingProtocol::RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,UnicastForwardCallback ucb,  					    MulticastForwardCallback mcb,LocalDeliverCallback lcb, ErrorCallback ecb)
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
				//NS_LOG_FUNCTION(this);
			
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
