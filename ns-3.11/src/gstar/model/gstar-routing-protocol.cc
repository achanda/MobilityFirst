/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//complete GSTAR protocol working

#include "gstar-routing-protocol.h"
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
#include <algorithm>
#include <limits>
#include <iostream>

#define GSTAR_PORT_NUMBER 698

/// \brief Gets the delay between a given time and the current time.
///
/// If given time is previous to the current one, then this macro returns
/// a number close to 0. This is used for scheduling events at a certain moment.
///
#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
                     (time - Simulator::Now () + Seconds (0.000001)))

using namespace std;

namespace ns3 
{

namespace gstar 
{
        NS_LOG_COMPONENT_DEFINE ("GstarRoutingProtocol");
        NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

//standard function used in AODV and OLSR codes
TypeId RoutingProtocol::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::gstar::RoutingProtocol")
    	.SetParent<Ipv4RoutingProtocol> ()
    	.AddConstructor<RoutingProtocol> ()
    	.AddAttribute ("HelloInterval", "HELLO messages emission interval.",
                   TimeValue (Seconds (2)),
                   MakeTimeAccessor (&RoutingProtocol::m_LP_Interval),
                   MakeTimeChecker ())
        .AddAttribute ("LSAInterval", "LSA messages emission interval.",
                   TimeValue (Seconds (3)),
                   MakeTimeAccessor (&RoutingProtocol::m_LSA_Interval),
                   MakeTimeChecker ());
  	return tid;
}

TypeId RoutingProtocol::GetInstanceTypeId (void) const
{
       return GetTypeId ();
}

RoutingProtocol::RoutingProtocol ()
       : m_LP_Timer (Timer::CANCEL_ON_DESTROY), m_LP_jitter_Timer (Timer::CANCEL_ON_DESTROY), m_LSA_Timer (Timer::CANCEL_ON_DESTROY),m_LSA_jitter_Timer (Timer::CANCEL_ON_DESTROY)
{
        LogComponentEnable("GstarRoutingProtocol",LOG_LEVEL_ALL);
        NS_LOG_DEBUG("Constructed gstar::RoutingProtocol");
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
  
	m_LP_Timer.SetFunction (&RoutingProtocol::LPTimerExpire, this);//intilize link probe timer to go to LPTimer Expire function when scheduled
        m_LSA_Timer.SetFunction (&RoutingProtocol::LSATimerExpire, this);//intilize LSA timer to go to LSATimer Expire function when scheduled
        m_LP_jitter_Timer.SetFunction (&RoutingProtocol::SendLP, this);//adds a random jitter in sending a link probe
        m_LSA_jitter_Timer.SetFunction (&RoutingProtocol::SendLSA, this);//adds a random jitter in sending a LSA

  	LPSequenceNumber = 0;//initilizing starting link probe sequence number to 0
        LSASequenceNumber = 0;//initilizing starting LSA sequence number to 0
        LP_jitter_flag = 0;
        LSA_jitter_flag = 0;

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
      		socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvGSTAR,  this));
      		if (socket->Bind (inetAddr))
        	{
        	  	NS_FATAL_ERROR ("Failed to bind() GSTAR socket");
        	}
      		socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
                
      		m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
	
      		canRungstar = true;
    	}

  	if(canRungstar)
    	{
      		LPTimerExpire ();
               LSATimerExpire ();
      	}
        NS_LOG_DEBUG ("GSTAR on node " << m_mainAddress << " started");

}

//receive and process packets
//same as what OLSR code does
void RoutingProtocol::RecvGSTAR (Ptr<Socket> socket)
{
        //NS_LOG_FUNCTION(this);
	Ptr<Packet> receivedPacket;
  	Address sourceAddress;
  	receivedPacket = socket->RecvFrom (sourceAddress);

  	InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
  	Ipv4Address senderIfaceAddr = inetSourceAddr.GetIpv4 ();
  	Ipv4Address receiverIfaceAddr = m_socketAddresses[socket].GetLocal ();
        //NS_LOG_DEBUG ("OLSR node " << m_mainAddress << " received a OLSR packet from "<< senderIfaceAddr << " to " << receiverIfaceAddr);

	Ptr<Packet> packet = receivedPacket;
       
  	gstar::MessageHeader gstarPacketHeader;
	packet->RemoveHeader (gstarPacketHeader);

	switch (gstarPacketHeader.GetMessageType ())
        {
        	case 1: //received link probe message
                {
			uint32_t LP_seq_no = gstarPacketHeader.GetSequenceNumber ();
			SendLPACK(receiverIfaceAddr, senderIfaceAddr, LP_seq_no);
			break;
                }
		case 2: //received link probe ACK message
                {
			uint32_t LP_ACK_seq_no = gstarPacketHeader.GetSequenceNumber ();
                        Ipv4Address neighbor_addr = gstarPacketHeader.GetMessageAddr ();
			ProcessLPACK(neighbor_addr, LP_ACK_seq_no);
                        break;
                }
                case 3: //received LSA message
                {
                        uint32_t LSA_rcvd_seq_no = gstarPacketHeader.GetSequenceNumber ();
                        Ipv4Address LSA_addr = gstarPacketHeader.GetMessageAddr ();
                        //process this LSA omly if its not my LSA
                        if (LSA_addr != m_mainAddress)
                                ProcessLSA(gstarPacketHeader,LSA_addr,LSA_rcvd_seq_no);
                        break;
                }
	}
}

//create and broadcast link probe packet
void RoutingProtocol::SendLP ()
{
        //cout << LP_jitter_flag << endl;
        //NS_LOG_FUNCTION(this);
        if (LP_jitter_flag == 0)
        {
                LP_jitter_flag = 1;
                //cout << LP_jitter_flag << endl;
                UniformVariable xlp (0,0.001);
                m_LP_jitter_Timer.Schedule (Seconds(xlp.GetValue()));// delays the sending of link probe
        }
        
        else        
        {
                //cout << LP_jitter_flag << endl;
	        Ptr<Packet> packet = Create<Packet> ();
	        gstar::MessageHeader header;
	        header.SetMessageType (1); //sets message type = 1 for link probe packet
	        header.SetSequenceNumber (LPSequenceNumber); //sets message sequence number
                header.SetMessageAddr(m_mainAddress); //sets message address = main address
	        packet->AddHeader(header); //add header to the packet
                cout << "node: " << m_mainAddress << "- Send LP Seq no: " << LPSequenceNumber << endl;
   
	        Time now = Simulator::Now (); //get current time

	        link_probe_history[LPSequenceNumber] = now; // saves current time at array postion corresponding to sequence number of teh link probe
  
	        //sends this packet on each interface with receiver address set to broadcast
	        for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    	        {
      	        	Ipv4Address bcast = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
      	        	i->first->SendTo (packet, 0, InetSocketAddress (bcast, GSTAR_PORT_NUMBER));
    	        }
        }
}

//sends a link probe ACK on receiving a link probe
void RoutingProtocol::SendLPACK (const Ipv4Address receiverIface,const Ipv4Address senderIface, uint32_t seq_no)
{
        //NS_LOG_FUNCTION(this);

	Ptr<Packet> packet = Create<Packet> ();
	gstar::MessageHeader header;
	header.SetMessageType (2);//sets message type = 2 for link probe ACK packet
	header.SetSequenceNumber (seq_no);//sets message sequence number = sequence number in link probe received
        header.SetMessageAddr(m_mainAddress);//sets message addr = main address
	packet->AddHeader(header);//add header to the packet
       
        cout << "node: " << m_mainAddress << "- LP Rcvd Seq no: " << seq_no << endl;

	Ptr<Socket> socket = FindSocketWithAddress (receiverIface); //gets the socket corresponding to the one on which link probe was received
  	NS_ASSERT (socket);
      
	//sends packet to the above socket with receiver address set to the sender address of link probe
  	socket->SendTo (packet, 0, InetSocketAddress (senderIface, GSTAR_PORT_NUMBER));
}

//create and broadcast LSA packet
void RoutingProtocol::SendLSA ()
{
       // cout << "In SendLSA" << endl;
        if (LSA_jitter_flag == 0)
        {
                LSA_jitter_flag = 1;
                //cout << LP_jitter_flag << endl;
                UniformVariable xlsa (0,0.001);
                m_LSA_jitter_Timer.Schedule (Seconds(xlsa.GetValue()));// delays the sending of LSA
        }
        
        else        
        {
                Ptr<Packet> packet = Create<Packet> ();

                gstar::MessageHeader msg;
                gstar::MessageHeader::LSA &lsa = msg.GetLSA ();

                int lsa_length = 0;

                NeighborSet curr_neighbor_list = m_state.NeighborTable();      
                for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)  
                {
                        lsa.neighborAddresses.push_back (it->neighborAddr); //add address of neighbor
                        lsa_length++;
                        lsa.neighborSETT.push_back (it->SETT); //add SETT of neighbor
                        int count = 0;
                        int64_t lett = 0;
                        //calculating LETT from past 10 SETTs
                        for (int j=0; j<10; j++)
                        {
                                if(it->SETT_array[j] != 0)       
                                {        
                                        lett = lett + it->SETT_array[j];
                                        count++;
                                }
                        }
                        lett = lett/count;
                        lsa.neighborLETT.push_back (lett); //add LETT of neighbor
                }
                
                lsa.length = lsa_length; //sets length of LSA message equal to the total number of neighbors
                if(lsa_length == 0)
                        return;
        
	        msg.SetMessageType (3); //sets message type = 3 for link probe packet
	        msg.SetSequenceNumber (LSASequenceNumber); //sets message sequence number
                msg.SetMessageAddr(m_mainAddress); //sets message address = main address
                packet->AddHeader(msg);//add header to the packet
        
                cout << "node: " << m_mainAddress << "- Send LSA Seq no: " << LSASequenceNumber << endl;

                //sends this packet on each interface with receiver address set to broadcast
                for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    	        {
      	        	Ipv4Address bcast = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
      	        	i->first->SendTo (packet, 0, InetSocketAddress (bcast, GSTAR_PORT_NUMBER));
    	        }
        }
}

       
//process link probe ACK and forms neighbor table
void RoutingProtocol::ProcessLPACK (const Ipv4Address neighbor_add, uint32_t seq_no)
{
        //creates the neighbor table with SETTs and LETTs
       // cout << "node: " << m_mainAddress << "- LP ACK Rcvd Seq no: " << seq_no << endl;
        Time rcvd = Simulator::Now ();    //current time   
        Time send = link_probe_history[seq_no]; //time at which link probe of this sequence number was send

        Time SETT = rcvd - send; //calculating SETT
       // cout << "node: " << m_mainAddress << "- SETT: " << SETT << endl;
        
        int64_t SETT_int = SETT.GetNanoSeconds();

        m_state.InsertNeighborTuple (neighbor_add, SETT_int,seq_no,rcvd); //insert this neighbor information in neighbor table  
        m_events.Track (Simulator::Schedule (DELAY (rcvd + GSTAR_NEIGHBOR_TIMER),
                                               &RoutingProtocol::NeighborTupleTimerExpire,
                                               this,
                                               neighbor_add)); //sechudle timer for deleting this entry

        //printing neighbor table (DEBUG)
        cout << "Neighbor Table at node " << m_mainAddress << endl;
        NeighborSet curr_neighbor_list = m_state.NeighborTable();

        for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
    	{
                cout << "IP:" << it->neighborAddr << endl;
                //cout << "ARRAY:" << endl;
                //for (int j=0; j<10; j++)
                  //      cout << it->SETT_array[j] << endl;
        }
                
}

//process LSA , re-broadcasts it (if new) and forms LSA table
void RoutingProtocol::ProcessLSA (gstar::MessageHeader lsaPacket,Ipv4Address addr, uint32_t seq_no)
{
         cout << "node: " << m_mainAddress << "- LSA Rcvd Seq no: " << seq_no << "from" << addr << endl;
         const gstar::MessageHeader::LSA &lsa = lsaPacket.GetLSA ();
        // cout << "Length: " << lsa.length << endl;

         int64_t *sett_array;
         int64_t *lett_array;
         Ipv4Address *addr_array;
         int j;

         sett_array = (int64_t*)malloc(lsa.length*sizeof(int64_t));
         lett_array = (int64_t*)malloc(lsa.length*sizeof(int64_t));
         addr_array = (Ipv4Address*)malloc(lsa.length*sizeof(Ipv4Address));

         j=0;
         // gets the neighbor addresses in LSA
         for (std::vector<Ipv4Address>::const_iterator i = lsa.neighborAddresses.begin (); i != lsa.neighborAddresses.end (); i++)
         {
               addr_array[j] = *i;
               j++;
         }
              
         j=0;
         // gets the neighbor SETTs in LSA
         for (std::vector<int64_t>::const_iterator i = lsa.neighborSETT.begin (); i != lsa.neighborSETT.end (); i++)
         {
               sett_array[j] = *i; 
               j++;
         }
        
         j=0;
         // gets the neighbor LETTs in LSA
         for (std::vector<int64_t>::const_iterator i = lsa.neighborLETT.begin (); i != lsa.neighborLETT.end (); i++)
         {
               lett_array[j] = *i;
               j++;
         }              
        
        Time rcvd = Simulator::Now (); // current time required for LSA expire time   
        
        int lsa_flag = m_state.InsertLSATuple (addr,sett_array,lett_array,addr_array,seq_no,lsa.length,rcvd,m_mainAddress);  
       
        m_events.Track (Simulator::Schedule (DELAY (rcvd + GSTAR_LSA_TIMER),
                                               &RoutingProtocol::LSATupleTimerExpire,
                                               this,
                                               addr));//schedule timer for deleting this entry

        if (lsa_flag == 1)
        {
                Ptr<Packet> packet = Create<Packet> ();
                packet->AddHeader(lsaPacket);//add header to the packet
                //sends this packet on each interface with receiver address set to broadcast
                for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    	        {
                        Ipv4Address bcast = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
                        i->first->SendTo (packet, 0, InetSocketAddress (bcast, GSTAR_PORT_NUMBER));
                }
        }

        //printing LSA table (DEBUG)
        //cout << "LSA Table at node " << m_mainAddress << endl;
        LSASet curr_lsa_list = m_state.LSATable();

        //for (LSASet::iterator it = curr_lsa_list.begin ();it != curr_lsa_list.end (); it++)
    	//{
          //      cout << "LSA of " << it->Addr << endl;               
            //    for( int j=0; j<it->size_lsa; j++)
              //  {
                //        cout << "IP:" << it->neighbor_Addr[j] << endl;
                  //      cout << "SETT:" << it->SETT[j] << endl;
                    //    cout << "LETT:" << it->LETT[j] << endl;
                //}
                
        //}

         //printing Forward table (DEBUG)
        forward_table m_forward_table_c = m_state.ForwardTable();
        cout << "Forward Table at node " << m_mainAddress << endl;
        int pl;
	for(forward_table::iterator iter1 = m_forward_table_c.begin(); iter1 != m_forward_table_c.end(); ++iter1)
	{
		cout << "destination" << iter1->destinationAddr << endl;
		cout << "sett" << iter1->sett_sum << endl;
		cout << "lett" << iter1->lett_sum << endl;
		cout << "next hop" << iter1->nexthopAddr << endl;
   		cout << "path length" << iter1->path_length << endl;
		pl=iter1->path_length;
		cout << "path" << endl;
                for(int d = pl-1 ; d >=0 ; d--)
			cout << iter1->path[d] << endl;
	}

                
         
}

//function called when timer for a neighbor entry expires
void RoutingProtocol::NeighborTupleTimerExpire (Ipv4Address neighbor_add)
{
        NeighborTuple* neighbor_entry = m_state.FindNeighborTuple (neighbor_add); //finds the neighbor tuple given the neighbor address
        
        //checks to see if the expiration time is reached
        if (neighbor_entry->neighbor_expiration_time < Simulator::Now ())
                //if expiration time is reached, deletes the neighbor entry
                m_state.EraseNeighborTuple (neighbor_add);
        else
                //if expiration time is not reached, re-schedules the timer
                m_events.Track (Simulator::Schedule (DELAY (neighbor_entry->neighbor_expiration_time),
                                               &RoutingProtocol::NeighborTupleTimerExpire,
                                               this,
                                               neighbor_add));

}

//function called when timer for a LSA entry expires
void RoutingProtocol::LSATupleTimerExpire (Ipv4Address addr)
{
        LSATuple* lsa_entry = m_state.FindLSATuple (addr); //finds the LSA tuple given the node address
        
        //checks to see if the expiration time is reached
        if (lsa_entry->lsa_expiration_time < Simulator::Now ())
                //if expiration time is reached, deletes the LSA entry
                m_state.EraseLSATuple (addr);
        else
                //if expiration time is not reached, re-schedules the timer
                m_events.Track (Simulator::Schedule (DELAY (lsa_entry->lsa_expiration_time),
                                               &RoutingProtocol::LSATupleTimerExpire,
                                               this,
                                               addr));
}


//callback funtion for sending link probe
void RoutingProtocol::LPTimerExpire ()
{
        //NS_LOG_FUNCTION(this);

        LP_jitter_flag = 0;
	SendLP ();

	LPSequenceNumber = LPSequenceNumber + 1; // increments link probe sequence number
      
        // and wraps arround at 10
	if (LPSequenceNumber >= 10)
		LPSequenceNumber = 0;

        m_LP_Timer.Schedule (m_LP_Interval);//reschedule timer
}

//callback funtion for sending LSA
void RoutingProtocol::LSATimerExpire ()
{
        //NS_LOG_FUNCTION(this);

        LSA_jitter_flag = 0;
	SendLSA ();

	LSASequenceNumber = LSASequenceNumber + 1; // increments LSA sequence number
        
        m_LSA_Timer.Schedule (m_LSA_Interval);//reschedule timer
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
