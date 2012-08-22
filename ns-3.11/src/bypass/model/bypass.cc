/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//MobilityFirst packet processing 

#include "bypass.h"
#include "bypass-header.h"
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
#include <map>
#include <sys/time.h>

#define NUM_PACKET 100
#define GSTAR_PORT_NUMBER 4210
using namespace std;

namespace ns3 
{
	NS_LOG_COMPONENT_DEFINE ("Bypass");
	NS_OBJECT_ENSURE_REGISTERED (Bypass);

	//standard function used in AODV and OLSR codes
	TypeId Bypass::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::Bypass")
			.SetParent<Ipv4RoutingProtocol> ()
			.AddConstructor<Bypass> ()
		    .AddAttribute ("ARQInterval", "ACK messages emission interval.",
					   TimeValue (Seconds (1)),
					   MakeTimeAccessor (&Bypass::m_ackTime),
					   MakeTimeChecker ())
			.AddAttribute ("PacketInterval", "Data messages emission interval.",
						TimeValue (Seconds (0)),
						MakeTimeAccessor (&Bypass::m_packetTime),
						MakeTimeChecker ())
			.AddAttribute ("ACK Required?", "ACK messages required or not",
						UintegerValue (0),
						MakeUintegerAccessor (&Bypass::m_ack),
						MakeUintegerChecker <uint32_t> ())
			.AddAttribute ("Block destination", "Destination for the block",
					   Ipv4AddressValue ("0.0.0.0"),
					   MakeIpv4AddressAccessor (&Bypass::m_bd),
					   MakeIpv4AddressChecker ())
			.AddAttribute ("Block source", "Destination for the block",
					   Ipv4AddressValue ("0.0.0.0"),
					   MakeIpv4AddressAccessor (&Bypass::m_bs),
					   MakeIpv4AddressChecker ());
		return tid;
	}

	TypeId Bypass::GetInstanceTypeId (void) const
	{
		return GetTypeId ();
	}

	Bypass::Bypass ()
	{
		m_prevSeq = 0;
		m_seqno = 1; //set sequence number to 1
		// set main address of the destination
		m_dest = Ipv4Address ("10.1.6.2");
		m_src = Ipv4Address ("10.1.1.1");
		
		
			// full-wireless.cc
		/*PerNodeTable one;
		PerNodeTable two;
		PerNodeTable three;
		PerNodeTable four;
		PerNodeTable five;
		PerNodeTable six;
		PerNodeTable seven;
		
		one[m_dest] = std::make_pair(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.2"));
		two[m_dest] = std::make_pair(Ipv4Address("10.1.1.2"), Ipv4Address("10.1.1.3"));
		three[m_dest] = std::make_pair(Ipv4Address("10.1.1.3"), Ipv4Address("10.1.1.4"));
		four[m_dest] = std::make_pair(Ipv4Address("10.1.1.4"), Ipv4Address("10.1.1.5"));
		five[m_dest] = std::make_pair(Ipv4Address("10.1.1.5"), Ipv4Address("10.1.1.6"));
		six[m_dest] = std::make_pair(Ipv4Address("10.1.1.6"), Ipv4Address("10.1.1.7"));
		seven[m_dest] = std::make_pair(Ipv4Address("0.0.0.0"), Ipv4Address("0.0.0.0"));
		
		two[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.1.2"), Ipv4Address("10.1.1.1"));
		three[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.1.3"), Ipv4Address("10.1.1.2"));
		four[Ipv4Address("10.1.1.3")] = std::make_pair(Ipv4Address("10.1.1.4"), Ipv4Address("10.1.1.3"));
			//four[Ipv4Address("10.1.2.2")] = std::make_pair(Ipv4Address("10.1.3.2"), Ipv4Address("10.1.3.1"));
		five[Ipv4Address("10.1.1.4")] = std::make_pair(Ipv4Address("10.1.1.5"), Ipv4Address("10.1.1.4"));
			//five[Ipv4Address("10.1.3.2")] = std::make_pair(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.1"));
		six[Ipv4Address("10.1.1.5")] = std::make_pair(Ipv4Address("10.1.1.6"), Ipv4Address("10.1.1.5"));
			//six[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.5.1"), Ipv4Address("10.1.4.2"));
		seven[Ipv4Address("10.1.1.6")] = std::make_pair(Ipv4Address("10.1.1.7"), Ipv4Address("10.1.1.6"));
			//seven[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.6.1"), Ipv4Address("10.1.6.2"));
		
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.1"),one));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.2"),two));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.3"),three));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.4"),four));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.5"),five));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.6"),six));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.7"),seven));*/
		
			// wired-new.cc
			// test.cc
		PerNodeTable one;
		PerNodeTable two;
		PerNodeTable three;
		PerNodeTable four;
		PerNodeTable five;
		PerNodeTable six;
		PerNodeTable seven;
		
		one[m_dest] = std::make_pair(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.2"));
		two[m_dest] = std::make_pair(Ipv4Address("10.1.2.1"), Ipv4Address("10.1.2.2"));
		three[m_dest] = std::make_pair(Ipv4Address("10.1.3.1"), Ipv4Address("10.1.3.2"));
		four[m_dest] = std::make_pair(Ipv4Address("10.1.4.1"), Ipv4Address("10.1.4.2"));
		five[m_dest] = std::make_pair(Ipv4Address("10.1.5.1"), Ipv4Address("10.1.5.2"));
		six[m_dest] = std::make_pair(Ipv4Address("10.1.6.1"), Ipv4Address("10.1.6.2"));
		seven[m_dest] = std::make_pair(Ipv4Address("0.0.0.0"), Ipv4Address("0.0.0.0"));
		
		two[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.1.2"), Ipv4Address("10.1.1.1"));
		three[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.2.2"), Ipv4Address("10.1.2.1"));
		three[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.2.2"), Ipv4Address("10.1.2.1"));
			//four[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.3.2"), Ipv4Address("10.1.3.1"));
		four[Ipv4Address("10.1.2.2")] = std::make_pair(Ipv4Address("10.1.3.2"), Ipv4Address("10.1.3.1"));
		four[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.3.2"), Ipv4Address("10.1.3.1"));
			//five[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.1"));
		five[Ipv4Address("10.1.3.2")] = std::make_pair(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.1"));
		five[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.1"));
		six[Ipv4Address("10.1.4.2")] = std::make_pair(Ipv4Address("10.1.5.2"), Ipv4Address("10.1.5.1"));
		six[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.5.2"), Ipv4Address("10.1.5.1"));
		seven[Ipv4Address("10.1.5.2")] = std::make_pair(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.6.1"));
		seven[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.6.1"));
		
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.1"),one));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.2"),two));
		rtable.insert(std::make_pair(Ipv4Address("10.1.2.2"),three));
		rtable.insert(std::make_pair(Ipv4Address("10.1.3.2"),four));
		rtable.insert(std::make_pair(Ipv4Address("10.1.4.2"),five));
		rtable.insert(std::make_pair(Ipv4Address("10.1.5.2"),six));
		rtable.insert(std::make_pair(Ipv4Address("10.1.6.2"),seven));
		
			// mixed-new.cc
		/*PerNodeTable one;
		PerNodeTable two;
		PerNodeTable three;
		PerNodeTable four;
		PerNodeTable five;
		PerNodeTable six;
		PerNodeTable seven;
		
		one[m_dest] = std::make_pair(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.2"));
		two[m_dest] = std::make_pair(Ipv4Address("10.1.2.1"), Ipv4Address("10.1.2.2"));
		three[m_dest] = std::make_pair(Ipv4Address("10.1.3.1"), Ipv4Address("10.1.3.2"));
		four[m_dest] = std::make_pair(Ipv4Address("10.1.4.1"), Ipv4Address("10.1.4.2"));
		five[m_dest] = std::make_pair(Ipv4Address("10.1.5.1"), Ipv4Address("10.1.5.2"));
		six[m_dest] = std::make_pair(Ipv4Address("10.1.5.2"), Ipv4Address("10.1.5.3"));
		seven[m_dest] = std::make_pair(Ipv4Address("0.0.0.0"), Ipv4Address("0.0.0.0"));
		
		two[Ipv4Address("10.1.1.1")] = std::make_pair(Ipv4Address("10.1.1.2"), Ipv4Address("10.1.1.1"));
		three[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.2.2"), Ipv4Address("10.1.2.1"));
		four[Ipv4Address("10.1.2.2")] = std::make_pair(Ipv4Address("10.1.3.2"), Ipv4Address("10.1.3.1"));
		four[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.3.2"), Ipv4Address("10.1.3.1"));
		five[Ipv4Address("10.1.3.2")] = std::make_pair(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.1"));
		five[Ipv4Address("10.1.1.2")] = std::make_pair(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.1"));
		six[Ipv4Address("10.1.4.2")] = std::make_pair(Ipv4Address("10.1.5.2"), Ipv4Address("10.1.5.1"));
		seven[Ipv4Address("10.1.5.2")] = std::make_pair(Ipv4Address("10.1.5.3"), Ipv4Address("10.1.5.2"));
		
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.1"),one));
		rtable.insert(std::make_pair(Ipv4Address("10.1.1.2"),two));
		rtable.insert(std::make_pair(Ipv4Address("10.1.2.2"),three));
		rtable.insert(std::make_pair(Ipv4Address("10.1.3.2"),four));
		rtable.insert(std::make_pair(Ipv4Address("10.1.4.2"),five));
		rtable.insert(std::make_pair(Ipv4Address("10.1.5.2"),six));
		rtable.insert(std::make_pair(Ipv4Address("10.1.5.3"),seven));*/
	}

	Bypass::~Bypass ()
	{
	}
	

	//standard function used in AODV and OLSR codes
	void Bypass::SetIpv4 (Ptr<Ipv4> ipv4)
	{
		NS_ASSERT (ipv4 != 0);
		NS_ASSERT (m_ipv4 == 0);
		NS_LOG_DEBUG("Starting Bypass SetIpv4");
		m_ackTimer.SetFunction (&Bypass::ACKTimerExpire,this);
		m_packetTimer.SetFunction(&Bypass::PacketTimerExpire, this);
		
		m_ipv4 = ipv4;
		queue_empty = 0;
	}

	//standard funtion used in AODV and OLSR codes
	void Bypass::DoDispose ()
	{
		m_ipv4 = 0;

		for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddresses.begin ();iter != m_socketAddresses.end (); iter++)
		{
			iter->first->Close ();
		}
		m_socketAddresses.clear ();
		Ipv4RoutingProtocol::DoDispose ();
	}

	void Bypass::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
	{

	}

	//creates sockets for each interface
	//exactly from OLSR code
	void Bypass::DoStart ()
	{	
		Ipv4Address loopback ("127.0.0.1");
		
		if (m_mainAddress == Ipv4Address ())
        {
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
		
			cout << "Starting Bypass on node " << m_mainAddress << endl;
		
		Ipv4Address addr;

		for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
		{
			addr = m_ipv4->GetAddress (i, 0).GetLocal ();
			if (addr == loopback)
				continue;

			// Create a socket to listen only on this interface
			Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),UdpSocketFactory::GetTypeId ());
			socket->SetAllowBroadcast (true);
			InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), GSTAR_PORT_NUMBER);
			socket->SetRecvCallback (MakeCallback (&Bypass::RecvMF,  this));
			if (socket->Bind (inetAddr))
			{
				NS_FATAL_ERROR ("Failed to bind() GSTAR socket");
			}
			socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
			m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
		}
		
			// full-wireless.cc
		/*if (m_mainAddress == "10.1.1.1")
		{
			m_ack = 1;
			m_bs = "10.1.1.1";
			m_bd = "10.1.1.2";
		}
		else if (m_mainAddress == "10.1.1.2")
		{
			m_ack = 1;
			m_bs = "10.1.1.2";
			m_bd = "10.1.1.3";
		}
		else if (m_mainAddress == "10.1.1.3")
		{
			m_ack = 1;
			m_bs = "10.1.1.3";
			m_bd = "10.1.1.4";
		}
		else if (m_mainAddress == "10.1.1.4")
		{
			m_ack = 1;
			m_bs = "10.1.1.4";
			m_bd = "10.1.1.5";
		}
		else if (m_mainAddress == "10.1.1.5")
		{
			m_ack = 1;
			m_bs = "10.1.1.5";
			m_bd = "10.1.1.6";
		}
		else if (m_mainAddress == "10.1.1.6")
		{
			m_ack = 1;
			m_bs = "10.1.1.6";
			m_bd = "10.1.1.7";
		}*/

			// change this to change topology
			// wired-new.cc
		if (m_mainAddress == "10.1.1.1")
		{
			m_ack = 1;
			m_bs = "10.1.1.1";
			m_bd = "10.1.1.2";
		}
		else if (m_mainAddress == "10.1.1.2")
		{
			m_ack = 1;
			m_bs = "10.1.1.2";
			m_bd = "10.1.2.2";
		}
		else if (m_mainAddress == "10.1.2.2")
		{
			m_ack = 1;
			m_bs = "10.1.2.2";
			m_bd = "10.1.3.2";
		}
		else if (m_mainAddress == "10.1.3.2")
		{
			m_ack = 1;
			m_bs = "10.1.3.2";
			m_bd = "10.1.4.2";
		}
		else if (m_mainAddress == "10.1.4.2")
		{
			m_ack = 1;
			m_bs = "10.1.4.2";
			m_bd = "10.1.5.2";
		}
		else if (m_mainAddress == "10.1.5.2")
		{
			m_ack = 1;
			m_bs = "10.1.5.2";
			m_bd = "10.1.6.2";
		}
		
			// mixed-new.cc (bypass)
		/*if (m_mainAddress == "10.1.1.1")
		{
			m_ack = 1;
			m_bs = "10.1.1.1";
			m_bd = "10.1.1.2";
		}
		else if (m_mainAddress == "10.1.1.2")
		{
			m_ack = 1;
			m_bs = "10.1.1.2";
			m_bd = "10.1.4.2";
		}
		else if (m_mainAddress == "10.1.4.2")
		{
			m_ack = 1;
			m_bs = "10.1.4.2";
			m_bd = "10.1.5.2";
		}
		else if (m_mainAddress == "10.1.5.2")
		{
			m_ack = 1;
			m_bs = "10.1.5.2";
			m_bd = "10.1.5.3";
		}*/
		
			// mixed-new.cc (hop)
		/*if (m_mainAddress == "10.1.1.1")
		{
			m_ack = 1;
			m_bs = "10.1.1.1";
			m_bd = "10.1.1.2";
		}
		else if (m_mainAddress == "10.1.1.2")
		{
			m_ack = 1;
			m_bs = "10.1.1.2";
			m_bd = "10.1.2.2";
		}
		else if (m_mainAddress == "10.1.2.2")
		{
			m_ack = 1;
			m_bs = "10.1.2.2";
			m_bd = "10.1.3.2";
		}
		else if (m_mainAddress == "10.1.3.2")
		{
			m_ack = 1;
			m_bs = "10.1.3.2";
			m_bd = "10.1.4.2";
		}
		else if (m_mainAddress == "10.1.4.2")
		{
			m_ack = 1;
			m_bs = "10.1.4.2";
			m_bd = "10.1.5.2";
		}
		else if (m_mainAddress == "10.1.5.2")
		{
			m_ack = 1;
			m_bs = "10.1.5.2";
			m_bd = "10.1.5.3";
		}*/
		
		if( addr == m_src) //this is the initial source, create packets and send to 10.1.1.2
		{
				m_packetTimer.Schedule (m_packetTime);
		}		
	}
	
	time_t Bypass::GetTime(void)
	{
		struct timeval t1;
		gettimeofday(&t1, NULL); 
		time_t cur = t1.tv_sec; //seconds
		time_t curr = t1.tv_usec; //microseconds
		
		time_t sett = cur * 1000000 + curr;
		return sett;
	}

	
	void Bypass::PacketTimerExpire(void)
	{
			for(int i=0; i<NUM_PACKET; i++)
			{
					//cout << "$$$$$" << GetTime() << endl;
			m_packet = Create<Packet> ();
			ns3::MessageHeader msg;
			msg.SetBS(m_mainAddress);
			msg.SetBD (Ipv4Address("10.1.1.2"));
			msg.SetDestination (m_dest);
			msg.SetType (1); // src data packet
			msg.SetSequence (m_seqno);
			m_seqno++;
			m_packet->AddHeader(msg);
		
			std::pair<Ipv4Address, Ipv4Address> temp = GetInterfaces(m_src, m_dest);
			my_iface = temp.first;
			neighbor_iface = temp.second;
		
			char buf[BUFFER_SIZE];
			for (int i=0;i<BUFFER_SIZE;i++)
			{
				char c;
				srand( (unsigned int) time(NULL));
				c = (rand() % 26) + 'a';
				buf[i] = c;
			}
		
			msg.SetBuffer(buf);
			data_pkts.push_back (m_packet);
					//cout << "%%%%" << GetTime() << endl;
		}
		
		m_packet = data_pkts.front();
		std::vector< Ptr<Packet> >::iterator it = data_pkts.begin();
		data_pkts.erase(it);
		
		Ptr<Socket> socketn = FindSocketWithAddress (my_iface);
		NS_ASSERT (socketn);
		socketn->SendTo (m_packet, 0, InetSocketAddress (neighbor_iface, GSTAR_PORT_NUMBER));
		cout << "Sending a packet, final destination: " << m_dest << endl;
		cout << "$$Packet sending starts at "<< GetTime() << endl;
		
		m_ackTimer.Schedule (m_ackTime);
	}

	//receive and process packets
	//same as what OLSR code does
	void Bypass::RecvMF (Ptr<Socket> socket)
	{
		Ptr<Packet> receivedPacket;
		Address sourceAddress;
		receivedPacket = socket->RecvFrom (sourceAddress);
		
		InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
		Ipv4Address senderIfaceAddr = inetSourceAddr.GetIpv4 ();
		Ipv4Address receiverIfaceAddr = m_socketAddresses[socket].GetLocal ();

		Ptr<Packet> packet = receivedPacket;
		ns3::MessageHeader msg;
		packet->RemoveHeader(msg);
				
		if (msg.GetType() == 1)
		{
			cout << "Data packet received by " << m_mainAddress << " BS: " << msg.GetBS() << " BD: " << msg.GetBD() << "seq_no" << msg.GetSequence() << endl;
				//cout << msg.GetBD() << endl;
		}
		else 
			cout << "Ack received by " << m_mainAddress << " BS: " << msg.GetBS() << " BD: " << msg.GetBD() << endl;

		
		switch (msg.GetType ())
        {
			// received data packet
        	case 1:
			{	
				if (msg.GetSequence() == m_prevSeq + 1)
				{
					m_prevSeq = msg.GetSequence();
					
					if (msg.GetBD() == m_mainAddress)
					{
						// if I am the block destination, I should send back and ack to block source
						Ptr<Packet> ACKpacket = Create<Packet> ();
						ns3::MessageHeader msgACK;

						msgACK.SetDestination (msg.GetBS());
						msgACK.SetType (2);
						msgACK.SetBS(Ipv4Address("0.0.0.0"));
						msgACK.SetBD (Ipv4Address("0.0.0.0"));
						
						ACKpacket->AddHeader(msgACK);
						std::pair<Ipv4Address, Ipv4Address> temp = GetInterfaces(m_mainAddress, msg.GetBS());
					
						Ptr<Socket> socketn_1 = FindSocketWithAddress (temp.first); //gets the socket corresponding to the one on which link probe was rcvd
						//Ptr<Socket> socketn_1 = FindSocketWithAddress ("10.1.1.2");
						NS_ASSERT (socketn_1);
					
						socketn_1->SendTo (ACKpacket, 0, InetSocketAddress (temp.second, GSTAR_PORT_NUMBER));
						cout << m_mainAddress << "sending ACK" << endl;
						//socketn_1->SendTo (ACKpacket, 0, InetSocketAddress ("10.1.1.1", GSTAR_PORT_NUMBER));
					}
                
					if (msg.GetDestination() != m_mainAddress) // if I am no the destination, forward
					{	
						cout << "reforwarding:" << m_mainAddress << endl;
						Ptr<Packet> q_packet = Create<Packet> ();
						msg.SetType(1);
					
						if (m_ack == 1)
						{
							msg.SetBS(m_mainAddress);
							cout << m_mainAddress << endl;
							cout << m_bd << endl;
							msg.SetBD(m_bd);
						}
					
						q_packet->AddHeader(msg);
					
						std::pair<Ipv4Address, Ipv4Address> temp = GetInterfaces(m_mainAddress, m_dest);
						my_iface = temp.first;
						neighbor_iface = temp.second;
					
						if(queue_empty == 0)
						{		
							cout << "queue" << endl;
							m_packet = q_packet;
							Ptr<Socket> socketn = FindSocketWithAddress (my_iface);
							cout << my_iface << endl;
							cout << neighbor_iface << endl;
							NS_ASSERT (socketn);
							socketn->SendTo (m_packet, 0, InetSocketAddress (neighbor_iface, GSTAR_PORT_NUMBER));
					
							if (m_ack == 1)
							{
								m_ackTimer.Schedule (m_ackTime);
								queue_empty = 1;
							}
						}
						else 
						data_pkts.push_back (q_packet);
					}
					else 
					{
						cout << "pkt finally rcvd at " << m_mainAddress << endl;
						cout << "**Packet sending ends at " << GetTime() << endl;
					}
				}
				break;
			}
			// received ack
			case 2:
			{					
				if (msg.GetDestination() == m_mainAddress)
				{
					m_ackTimer.Cancel();
					queue_empty = 0;
					if (data_pkts.size() != 0)
					{
						m_packet = data_pkts.front();
						std::vector< Ptr<Packet> >::iterator it = data_pkts.begin();
						data_pkts.erase(it);
						Ptr<Socket> socketn = FindSocketWithAddress (my_iface);
						
						NS_ASSERT (socketn);
						socketn->SendTo (m_packet, 0, InetSocketAddress (neighbor_iface, GSTAR_PORT_NUMBER));
						
						if(m_mainAddress == m_src)
						{
							cout << "$$Packet sending starts at " << GetTime() << endl;
						}
						
						queue_empty = 1;
				
						m_ackTimer.Schedule (m_ackTime);
					}
				}
				else 
				{
						// send back ack
					cout << "Reforwarding ack " << m_mainAddress << endl;
					Ptr<Packet> q_packet = Create<Packet> ();
					q_packet->AddHeader(msg);
					std::pair<Ipv4Address, Ipv4Address> temp = GetInterfaces(m_mainAddress, msg.GetDestination());
					
					Ptr<Socket> socketn = FindSocketWithAddress (temp.first); //gets the socket corresponding to the one on which link probe was rcvd
					NS_ASSERT (socketn);
					
					socketn->SendTo (q_packet, 0, InetSocketAddress (temp.second, GSTAR_PORT_NUMBER));
				}

				break;
			}	
		}
	}
	
	void  Bypass::ACKTimerExpire()
	{
		cout << "timer expire " << m_mainAddress << endl;
		Ptr<Socket> socketn = FindSocketWithAddress (my_iface);
		NS_ASSERT (socketn);
		socketn->SendTo (m_packet, 0, InetSocketAddress (neighbor_iface, GSTAR_PORT_NUMBER));
		cout << "Sending a packet, final destination: " << m_dest << endl;
		
		m_ackTimer.Schedule (m_ackTime);
	}

	//everything from here is same as in OLSR code
	Ptr<Ipv4Route> Bypass::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
	{
		NS_LOG_FUNCTION(this);

		Ptr<Ipv4Route> rtentry;
		return rtentry;
	}

	bool Bypass::RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,UnicastForwardCallback ucb,MulticastForwardCallback mcb,LocalDeliverCallback lcb, ErrorCallback ecb)
	{
		NS_LOG_FUNCTION(this);

		return false;
	}

	void Bypass::NotifyInterfaceUp (uint32_t i)
	{

	}

	void Bypass::NotifyInterfaceDown (uint32_t i)
	{

	}

	void Bypass::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
	{

	}

	void Bypass::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
	{

	}

	bool Bypass::IsMyOwnAddress (const Ipv4Address & a) const
	{			
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
	Ptr<Socket> Bypass::FindSocketWithAddress (const Ipv4Address addr) const
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
	
	//std::map<Ipv4Address, Ipv4Address> 
	std::pair<Ipv4Address, Ipv4Address> Bypass::GetInterfaces(Ipv4Address mainAddress, Ipv4Address dest)
	{
		std::map<Ipv4Address, PerNodeTable>::iterator it = rtable.find(mainAddress);
		
		PerNodeTable table = it->second;
		std::map<Ipv4Address, std::pair<Ipv4Address, Ipv4Address> >::iterator itt = table.find(dest);
		
		std::pair<Ipv4Address, Ipv4Address> mpair = itt->second;
		return mpair;
	}
}
