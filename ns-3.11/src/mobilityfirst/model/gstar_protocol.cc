//functions for handling GSTAR packets and tables

#include "gstar_protocol.h"
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

#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
                     (time - Simulator::Now () + Seconds (0.000001)))

using namespace std;

namespace ns3 
{

	GSTARProtocol::GSTARProtocol()
        {
		 
		 LPSequenceNumber = 0;//initilizing starting link probe sequence number to 0
		 LSASequenceNumber = 0;//initilizing starting LSA sequence number to 0
		 SummVecSequenceNumber = 0;//initilizing starting summary vector sequence number to 0
		 DLSASequenceNumber = 0;//initilizing starting DLSA sequence number to 0
		 rebroadcast_lsa_pkt = Create<Packet> ();
	        
	}

	//creates link probe packet
	Ptr<Packet> GSTARProtocol::SendLP (Ipv4Address m_mainAddress, uint8_t mac_addr[])
	{
		Ptr<Packet> packet = Create<Packet> ();
        	mf::MessageHeader header;

		mf::MessageHeader::LP &lp = header.GetLP ();
		for (int i=0; i<6; i++)
			lp.my_mac_addr[i] = mac_addr[i];

		 Mac48Address m_mac_addr_1;
                 m_mac_addr_1.CopyFrom(lp.my_mac_addr);
                 cout  << m_mainAddress << " " << m_mac_addr_1 << endl;

	       	header.SetMessageType (1); //sets message type = 1 for link probe packet
	       	header.SetSequenceNumber (LPSequenceNumber); //sets message sequence number
               	header.SetMessageAddr(m_mainAddress); //sets message address = main address

	       	packet->AddHeader(header); //add header to the packet
                
	        Time now = Simulator::Now (); //get current time
		link_probe_history[LPSequenceNumber] = now; // saves current time at array postion corresponding to sequence number of teh link probe
		
		LPSequenceNumber++;
		if (LPSequenceNumber >= 10)
			LPSequenceNumber = 0; 
		
		return packet;
		
	}

	//creates link probe ACK packet
	Ptr<Packet> GSTARProtocol::SendLPACK (const Ipv4Address receiverIface,const Ipv4Address senderIface, uint32_t seq_no, Ipv4Address m_mainAddress)
	{
        	//NS_LOG_FUNCTION(this);
	
		Ptr<Packet> packet = Create<Packet> ();//sends link probe
		mf::MessageHeader header;
		header.SetMessageType (2);//sets message type = 2 for link probe ACK packet
		header.SetSequenceNumber (seq_no);//sets message sequence number = sequence number in link probe received
        	header.SetMessageAddr(m_mainAddress);//sets message addr = main address
		packet->AddHeader(header);//add header to the packet

		

		return packet;
       }

	int GSTARProtocol::ProcessLP (mf::MessageHeader lpPacket, Ptr<WifiNetDevice> m_wifi, Ipv4Address my_IfaceAddr, Ipv4Address sender_IfaceAddr,Ipv4Address m_mainAddress)
	{
		const mf::MessageHeader::LP &lp = lpPacket.GetLP ();
		Mac48Address neighbor_mac_addr;
                neighbor_mac_addr.CopyFrom(lp.my_mac_addr);
                
		WifiMacHeader m_testHeader; 
		m_testHeader.SetDsFrom ();
                m_testHeader.SetDsTo ();
                m_testHeader.SetTypeData ();
                
		Ptr<Packet> m_testFrame = Create<Packet> (30);                       
                            
		int64_t SETT;                                                   
               	if(m_wifi != NULL)
		{
			WifiMode mode = m_wifi->GetRemoteStationManager()->GetDataMode(neighbor_mac_addr, &m_testHeader, m_testFrame, m_testFrame->GetSize ());
                	//cout << "Mode " << mode << endl;
                	SETT = mode.GetDataRate();
                	//cout << "Rate " << SETT << endl;
		}
		else
			SETT = 54000000;

		uint32_t seq_no = lpPacket.GetSequenceNumber ();
                Ipv4Address neighbor_add = lpPacket.GetMessageAddr ();

		int new_neighbor_flag = 0;
		Time rcvd = Simulator::Now ();    //current time 

		new_neighbor_flag = m_state_gstar.InsertNeighborTuple (neighbor_add, (45*8.0*1000000000)/SETT, seq_no, rcvd, my_IfaceAddr, sender_IfaceAddr,m_mainAddress); //insert this neighbor information in neighbor table  
    		m_events_gstar.Track (Simulator::Schedule (DELAY (rcvd + GSTAR_NEIGHBOR_TIMER),
        	                                     &GSTARProtocol::NeighborTupleTimerExpire,
            		                             this,
                	                             neighbor_add)); //sechudle timer for deleting this entry


		cout << "Neighbor Table at node " << m_mainAddress << endl;
		
                NeighborSet curr_neighbor_list = m_state_gstar.NeighborTable();
		
		for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
    	        {
                          cout << "IP:" << it->neighborAddr << endl;
			  //cout << "My Iterface:" << it->my_ifac << endl;
		          //cout << "Neighbor Interface:" << it->neighbor_iface << endl;
			  cout << "Current SETT " << it->SETT << endl;
			  cout << "Average SETT " << it->SETT_avg << endl;
                          //cout << "ARRAY:" << endl;
                          //for (int j=0; j<10; j++)
                          //cout << it->SETT_array[j] << endl;
			  cout << "LETT " << it->LETT << endl;
			
			  cout  << endl;
                }
                
		return new_neighbor_flag;
	}

	//process link probe ACK and forms neighbor table
	int GSTARProtocol::ProcessLPACK (const Ipv4Address neighbor_add, uint32_t seq_no, Ipv4Address m_mainAddress, Ipv4Address my_IfaceAddr, Ipv4Address sender_IfaceAddr)
	{
		//cout << "in LPACK process" << endl;
		int new_neighbor_flag = 0;
        	Time rcvd = Simulator::Now ();    //current time   
        	Time send = link_probe_history[seq_no]; //time at which link probe of this sequence number was send

        	Time SETT = rcvd - send; //calculating SETT
		
		int64_t SETT_int = SETT.GetNanoSeconds();

		new_neighbor_flag = m_state_gstar.InsertNeighborTuple (neighbor_add, SETT_int/2,seq_no,rcvd,my_IfaceAddr,sender_IfaceAddr,m_mainAddress); //insert this neighbor information in neighbor table  
    		m_events_gstar.Track (Simulator::Schedule (DELAY (rcvd + GSTAR_NEIGHBOR_TIMER),
        	                                     &GSTARProtocol::NeighborTupleTimerExpire,
            		                             this,
                	                             neighbor_add)); //sechudle timer for deleting this entry

		cout << "Neighbor Table at node " << m_mainAddress << endl;
		cout << SETT << endl;
                NeighborSet curr_neighbor_list = m_state_gstar.NeighborTable();

                for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
    	        {
                          cout << "IP:" << it->neighborAddr << endl;
			  //cout << "My Iterface:" << it->my_ifac << endl;
		          //cout << "Neighbor Interface:" << it->neighbor_iface << endl;
			  cout << "Current SETT " << it->SETT << endl;
			  cout << "Average SETT " << it->SETT_avg << endl;
                          //cout << "ARRAY:" << endl;
                          //for (int j=0; j<10; j++)
                            //     cout << it->SETT_array[j] << endl;

			  cout << "LETT " << it->LETT << endl;
			
			  cout  << endl;
                }
                
		return new_neighbor_flag;
	}

	//creates LSA packet
	Ptr<Packet> GSTARProtocol::SendLSA (Ipv4Address m_mainAddress)
	{
       		Ptr<Packet> packet = Create<Packet> ();

                mf::MessageHeader msg;
                mf::MessageHeader::LSA &lsa = msg.GetLSA ();

                int lsa_length = 0;

                NeighborSet curr_neighbor_list = m_state_gstar.NeighborTable();      
                for (NeighborSet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)  
                {
                        lsa.neighborAddresses.push_back (it->neighborAddr); //add address of neighbor
                        lsa_length++;
                        lsa.neighborSETT.push_back (it->SETT_avg); //add SETT of neighbor
                        lsa.neighborLETT.push_back (it->LETT); //add LETT of neighbor
                }
                
                lsa.length = lsa_length; //sets length of LSA message equal to the total number of neighbors
                
   		msg.SetMessageType (3); //sets message type = 3 for link probe packet
	        msg.SetSequenceNumber (LSASequenceNumber); //sets message sequence number
                msg.SetMessageAddr(m_mainAddress); //sets message address = main address
                packet->AddHeader(msg);//add header to the packet
        
		//cout << "LSA send " << LSASequenceNumber << "of length " << lsa.length << "by node " <<  m_mainAddress << endl;

		LSASequenceNumber++;
		
		return packet;
      	}
	
	//process LSA , re-broadcasts it (if new) and forms LSA table
	int GSTARProtocol::ProcessLSA (mf::MessageHeader lsaPacket,Ipv4Address addr, uint32_t seq_no, Ipv4Address m_mainAddress)
	{
         	cout << "node" << m_mainAddress << "LSA Rcvd Seq no: " << seq_no << "from" << addr << endl;
         	const mf::MessageHeader::LSA &lsa = lsaPacket.GetLSA ();
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
        
        	int lsa_flag = m_state_gstar.InsertLSATuple (addr,sett_array,lett_array,addr_array,seq_no,lsa.length,rcvd,m_mainAddress);  

		if (lsa_flag == 1)
                	rebroadcast_lsa_pkt->AddHeader(lsaPacket);//add header to the packet
       
        	m_events_gstar.Track (Simulator::Schedule (DELAY (rcvd + GSTAR_LSA_TIMER),
                                               &GSTARProtocol::LSATupleTimerExpire,
                                               this,
                                               addr));//schedule timer for deleting this entry

       		//printing LSA table (DEBUG)
        	//cout << "LSA Table at node " << m_mainAddress << endl;
        	//LSASet curr_lsa_list = m_state_gstar.LSATable();

        	//for (LSASet::iterator it = curr_lsa_list.begin ();it != curr_lsa_list.end (); it++)
    		//{
          	     //cout << "LSA of " << it->Addr << endl;               
            	     //for( int j=0; j<it->size_lsa; j++)
              	    //{
                        //cout << "IP:" << it->neighbor_Addr[j] << endl;
                        //cout << "SETT:" << it->SETT[j] << endl;
                        //cout << "LETT:" << it->LETT[j] << endl;
               	    //}
                //}

         	//printing Forward table (DEBUG)
        	forward_table m_forward_table_c = m_state_gstar.ForwardTable();
        	cout << "Forward Table at node " << m_mainAddress << endl;
        	//int pl;
		int i = 1;
		int flag;
		for(forward_table::iterator iter1 = m_forward_table_c.begin(); iter1 != m_forward_table_c.end(); ++iter1)
		{
			cout << i << endl;
			cout << "destination" << iter1->destinationAddr << endl;
			cout << "sett" << iter1->sett_sum << endl;
			cout << "lett" << iter1->lett_sum << endl;
		//	cout << "next hop" << iter1->nexthopAddr << endl;
   		//	cout << "path length" << iter1->path_length << endl;
		//	i++;
		//	pl=iter1->path_length;
		//	cout << "path" << endl;
                //	for(int d = pl-1 ; d >=0 ; d--)
		//		cout << iter1->path[d] << endl;
			flag =  m_state_gstar.forward_flag(iter1->destinationAddr);
			cout << "Flag " << flag << endl;
		}

		return lsa_flag;
	}

	//creates Summary Vector packet
	Ptr<Packet> GSTARProtocol::SendSummaryVector(Ipv4Address m_mainAddress)
	{
		//cout << "node " << m_mainAddress << " in SendSummaryVector" << endl;
       		Ptr<Packet> packet = Create<Packet> ();

                mf::MessageHeader msg;
                mf::MessageHeader::summary_vector &summ_vec = msg.Get_summary_vector ();

                int vec_length = 0;

                DLSASet curr_DLSA_list = m_state_gstar.DLSATable();      
                for (DLSASet::iterator it = curr_DLSA_list.begin ();it != curr_DLSA_list.end (); it++)  
                {
                        summ_vec.nodeAddresses.push_back (it->nodeAddr); //add address of node
			summ_vec.nodeseqnums.push_back (it->seq_no); //add sequence number of the DLSA for this node
                        vec_length++;
                       
                }

		//summ_vec.nodeAddresses.push_back (m_mainAddress);
		summ_vec.nodeseqnums.push_back (SummVecSequenceNumber);
                
                summ_vec.length = vec_length; //sets length of summary vector message equal to the total number of nodes
		
	        msg.SetMessageType (4); //sets message type = 4 for summary vector packet
	        msg.SetSequenceNumber (SummVecSequenceNumber); //sets message sequence number
                msg.SetMessageAddr(m_mainAddress); //sets message address = main address
                packet->AddHeader(msg);//add header to the packet
        
		////cout << "LSA send " << LSASequenceNumber << "of length " << lsa.length << "by node " <<  m_mainAddress << endl;

		SummVecSequenceNumber++;
		
		return packet;
      	}

	//creates Summary Vector ACK packet
	Ptr<Packet> GSTARProtocol::SendSummaryVectorACK(mf::MessageHeader summ_vec_pkt, uint32_t seq_no, Ipv4Address m_mainAddress)
	{
		//cout << "node " << m_mainAddress << " in SendSummaryVectorACK" << endl; 		
		const mf::MessageHeader::summary_vector &summ_vec = summ_vec_pkt.Get_summary_vector ();
				
		int64_t *seq_no_array;
         	Ipv4Address *addr_array;
         	int j;

         	seq_no_array = (int64_t*)malloc(summ_vec.length*sizeof(int64_t));
         	addr_array = (Ipv4Address*)malloc(summ_vec.length*sizeof(Ipv4Address));

		j=0;
         	// gets the node addresses in summary vector
         	for (std::vector<Ipv4Address>::const_iterator i = summ_vec.nodeAddresses.begin (); i != summ_vec.nodeAddresses.end (); i++)
         	{
               		addr_array[j] = *i;
               		j++;
         	}
              
         	j=0;
         	// gets the sequence numbers in summmary vector
         	for (std::vector<int64_t>::const_iterator i =summ_vec.nodeseqnums.begin (); i != summ_vec.nodeseqnums.end (); i++)
         	{
               		seq_no_array[j] = *i; 
               		j++;
         	}

		Ptr<Packet> packet = Create<Packet> ();

                mf::MessageHeader msg;
                mf::MessageHeader::summary_vector &summ_vec_ACK = msg.Get_summary_vector ();

                int vec_length = 0;

		for (int i=0; i<j; i++)  
                {
			bool dlsa_flag =  m_state_gstar.FindDLSA(addr_array[i],seq_no_array[i]);
			if(dlsa_flag == false && addr_array[i] != m_mainAddress)
			{
				summ_vec_ACK.nodeAddresses.push_back (addr_array[i]); //add address of node
				summ_vec_ACK.nodeseqnums.push_back (seq_no_array[i]); //add sequence number of the DLSA for this node
				vec_length++;
			}
		}
		
		summ_vec_ACK.length = vec_length;

		msg.SetMessageType (5); //sets message type = 5 for summary vector ACK packet
	        msg.SetSequenceNumber (seq_no); //sets message sequence number
                msg.SetMessageAddr(m_mainAddress); //sets message address = main address
                packet->AddHeader(msg);//add header to the packet

		return packet;
      	}

	//creates DLSA packet
	Ptr<Packet> GSTARProtocol::SendDLSA(Ipv4Address Addr, Ipv4Address m_mainAddress)
	{
		//cout << "node " << m_mainAddress << " in SendDLSA" << endl; 	
		Ptr<Packet> packet = Create<Packet> ();

                mf::MessageHeader msg;
                mf::MessageHeader::DLSA &dlsa = msg.GetDLSA ();

           	DLSATuple* curr_DLSA = m_state_gstar.FindDLSATuple (Addr);
		for(int i=0; i<curr_DLSA->size_dlsa; i++)
		{
			dlsa.nodeAddresses.push_back (curr_DLSA->Addr[i]); //add address of node
			dlsa.probabilities.push_back (curr_DLSA->probability[i]); //add probability of node
		}
		
		dlsa.length = curr_DLSA->size_dlsa; //sets length of LSA message equal to the total number of neighbors
		msg.SetSequenceNumber (curr_DLSA->seq_no); //sets message sequence number
		msg.SetMessageAddr(Addr); //sets message address = node address
		
		msg.SetMessageType (6); //sets message type = 6 for link probe packet
	        
                packet->AddHeader(msg);//add header to the packet

		return packet;
      	}

	Ptr<Packet> GSTARProtocol::SendMyDLSA(Ipv4Address m_mainAddress)
	{
		Ptr<Packet> packet = Create<Packet> ();

                mf::MessageHeader msg;
                mf::MessageHeader::DLSA &dlsa = msg.GetDLSA ();

                int dlsa_length = 0;

		NeighborProbabilitySet curr_neighbor_prob_list = m_state_gstar.NeighborProbabilityTable();      
                for (NeighborProbabilitySet::iterator it = curr_neighbor_prob_list.begin ();it != curr_neighbor_prob_list.end (); it++)  
                {
                	dlsa.nodeAddresses.push_back (it->neighborAddr); //add address of node
                       	dlsa_length++;
                       	dlsa.probabilities.push_back ((it->on_time*100)/(it->on_time + it->off_time)); //add probability of node
                }
		dlsa.length = dlsa_length;
		msg.SetSequenceNumber (DLSASequenceNumber); //sets message sequence number
		msg.SetMessageAddr(m_mainAddress);

		msg.SetMessageType (6); //sets message type = 6 for link probe packet
	        
                packet->AddHeader(msg);//add header to the packet

		DLSASequenceNumber++;
	
		return packet;
	}

		

	//process DLSA and forms DLSA table
	int GSTARProtocol::ProcessDLSA (mf::MessageHeader dlsaPacket,Ipv4Address addr, uint32_t seq_no, Ipv4Address m_mainAddress)
	{
         	cout << "node" << m_mainAddress << "DLSA Rcvd Seq no: " << seq_no << "from" << addr << endl;
         	const mf::MessageHeader::DLSA &dlsa = dlsaPacket.GetDLSA ();
        	// cout << "Length: " << dlsa.length << endl;

	        int64_t *probability_array;
        	Ipv4Address *addr_array;
         	int j;

         	probability_array = (int64_t*)malloc(dlsa.length*sizeof(int64_t));
         	addr_array = (Ipv4Address*)malloc(dlsa.length*sizeof(Ipv4Address));

         	j=0;
         	// gets the node addresses in DLSA
         	for (std::vector<Ipv4Address>::const_iterator i = dlsa.nodeAddresses.begin (); i != dlsa.nodeAddresses.end (); i++)
         	{
               		addr_array[j] = *i;
               		j++;
         	}
              
         	j=0;
         	// gets the node probabilities in DLSA
         	for (std::vector<int64_t>::const_iterator i = dlsa.probabilities.begin (); i != dlsa.probabilities.end (); i++)
         	{
               		probability_array[j] = *i; 
               		j++;
         	}
        
         	Time rcvd = Simulator::Now (); // current time required for DLSA expire time   
        
        	int dlsa_flag = m_state_gstar.InsertDLSATuple (addr,probability_array,addr_array,seq_no,dlsa.length,rcvd,m_mainAddress);  

		m_events_gstar.Track (Simulator::Schedule (DELAY (rcvd + GSTAR_DLSA_TIMER),
                                               &GSTARProtocol::DLSATupleTimerExpire,
                                               this,
                                               addr));//schedule timer for deleting this entry

       		//printing DLSA table (DEBUG)
        	//cout << "DLSA Table at node " << m_mainAddress << endl;
        	//DLSASet curr_dlsa_list = m_state_gstar.DLSATable();

        	//for (DLSASet::iterator it = curr_dlsa_list.begin ();it != curr_dlsa_list.end (); it++)
    		//{
          	  //   cout << "DLSA of " << it->nodeAddr << endl;               
            	   //  for( int j=0; j<it->size_dlsa; j++)
              	    // {
                     //   cout << "IP:" << it->Addr[j] << endl;
                     //   cout << "Probability:" << it->probability[j] << endl;
                     //}
                //}

         	//printing DTN Forward table (DEBUG)
		
		forward_table m_forward_table_c = m_state_gstar.ForwardTable();
		cout << "Forward Table at node " << m_mainAddress << endl;
        	//int pl;
		int i = 1;
		int flag;
		for(forward_table::iterator iter1 = m_forward_table_c.begin(); iter1 != m_forward_table_c.end(); ++iter1)
		{
			cout << i << endl;
			cout << "destination" << iter1->destinationAddr << endl;
			cout << "sett" << iter1->sett_sum << endl;
			cout << "lett" << iter1->lett_sum << endl;
				//	cout << "next hop" << iter1->nexthopAddr << endl;
				//	cout << "path length" << iter1->path_length << endl;
				//	i++;
				//	pl=iter1->path_length;
				//	cout << "path" << endl;
                //	for(int d = pl-1 ; d >=0 ; d--)
				//		cout << iter1->path[d] << endl;
			flag =  m_state_gstar.forward_flag(iter1->destinationAddr);
			cout << "Flag " << flag << endl;
		}
		
        	dtn_forward_table m_dtn_forward_table_c = m_state_gstar.DTNForwardTable();
        	cout << "DTN Forward Table at node " << m_mainAddress << endl;
        	int pl;
		for(dtn_forward_table::iterator iter1 = m_dtn_forward_table_c.begin(); iter1 != m_dtn_forward_table_c.end(); ++iter1)
		{
			cout << "destination" << iter1->destinationAddr << endl;
			cout << "probability" << iter1->probability << endl;
		 	cout << "next hop" << iter1->nexthopAddr << endl;
   			cout << "path length" << iter1->path_length << endl;
			pl=iter1->path_length;
			cout << "path" << endl;
                	for(int d = pl-1 ; d >=0 ; d--)
				cout << iter1->path[d] << endl;
		}

		return dlsa_flag;
		
	}

		
	//function called when timer for a neighbor entry expires
	void GSTARProtocol::NeighborTupleTimerExpire (Ipv4Address neighbor_add)
	{
        	NeighborTuple* neighbor_entry = m_state_gstar.FindNeighborTuple (neighbor_add); //finds the neighbor tuple given the neighbor address
        
		if(neighbor_entry != NULL)
		{
        		//checks to see if the expiration time is reached
        		if (neighbor_entry->neighbor_expiration_time < Simulator::Now ())
        	        	//if expiration time is reached, deletes the neighbor entry
        	        	m_state_gstar.EraseNeighborTuple (neighbor_add);
        		else//functions for handling GSTAR-Intrapartition packets and tables
        	        	//if expiration time is not reached, re-schedules the timer
        	        	m_events_gstar.Track (Simulator::Schedule (DELAY (neighbor_entry->neighbor_expiration_time),
                                              &GSTARProtocol::NeighborTupleTimerExpire,
                                              this,
                                              neighbor_add));
		}
	}

	//function called when timer for a LSA entry expires
	void GSTARProtocol::LSATupleTimerExpire (Ipv4Address addr)
	{
	        LSATuple* lsa_entry = m_state_gstar.FindLSATuple (addr); //finds the LSA tuple given the node address
	        
	        //checks to see if the expiration time is reached
		if(lsa_entry != NULL)
		{
	        	if (lsa_entry->lsa_expiration_time < Simulator::Now ())
	                	//if expiration time is reached, deletes the LSA entry
	                	m_state_gstar.EraseLSATuple (addr);
	       	 	else	
	                	//if expiration time is not reached, re-schedules the timer
	                	m_events_gstar.Track (Simulator::Schedule (DELAY (lsa_entry->lsa_expiration_time),
                                               &GSTARProtocol::LSATupleTimerExpire,
                                               this,
                                               addr));
		}
	}

	//function called when timer for a DLSA entry expires
	void GSTARProtocol::DLSATupleTimerExpire (Ipv4Address addr)
	{
	        DLSATuple* dlsa_entry = m_state_gstar.FindDLSATuple (addr); //finds the LSA tuple given the node address
		
	       	//checks to see if the expiration time is reached
		if(dlsa_entry != NULL)
		{
	        	if (dlsa_entry->dlsa_expiration_time < Simulator::Now ())
	                	//if expiration time is reached, deletes the DLSA entry
	                	m_state_gstar.EraseDLSATuple (addr);
	        	else
	                	//if expiration time is not reached, re-schedules the timer
	                	m_events_gstar.Track (Simulator::Schedule (DELAY (dlsa_entry->dlsa_expiration_time),
                                               &GSTARProtocol::DLSATupleTimerExpire,
                                               this,
                                               addr));
		}
	}

	//gives lsa pkt to be re-broadcasted
	Ptr<Packet> GSTARProtocol::GetPkt()
	{
		return rebroadcast_lsa_pkt;
	}

	// gives sender and receiver interfaces given the destination address
	InterfaceTuple GSTARProtocol::GetInterfaces(Ipv4Address Addr)
	{
			//printing DTN Forward table (DEBUG)
		dtn_forward_table m_dtn_forward_table_c = m_state_gstar.DTNForwardTable();
		cout << "DTN Forward Table at node " << endl;
		int pl;
		for(dtn_forward_table::iterator iter1 = m_dtn_forward_table_c.begin(); iter1 != m_dtn_forward_table_c.end(); ++iter1)
		{
			cout << "destination" << iter1->destinationAddr << endl;
			cout << "probability" << iter1->probability << endl;
		 	cout << "next hop" << iter1->nexthopAddr << endl;
   			cout << "path length" << iter1->path_length << endl;
			pl=iter1->path_length;
			cout << "path" << endl;
			for(int d = pl-1 ; d >=0 ; d--)
				cout << iter1->path[d] << endl;
		}
		
		
		Ipv4Address next_hop;
		//int dtn_flag = 0;
		next_hop = m_state_gstar.getNextHopIP (Addr);

		if (next_hop == "0.0.0.0")
		{
			next_hop = m_state_gstar.getDTNNextHopIP (Addr);
			//dtn_flag = 1;
			//cout << "next hop by DTN" << next_hop << endl;
		}
		
		InterfaceTuple interfaces;
		if (next_hop == "0.0.0.0")
		{
			interfaces.my_interface = "0.0.0.0";
			interfaces.neighbor_interface = "0.0.0.0";	
		}
		else
		{	
			NeighborTuple* tuple = m_state_gstar.FindNeighborTuple (next_hop);

			if(tuple != NULL)
			{
				interfaces.my_interface = tuple->my_ifac;
				interfaces.neighbor_interface = tuple->neighbor_iface;
			}
			else
			{
				interfaces.my_interface = "0.0.0.0";
				interfaces.neighbor_interface = "0.0.0.0";	
			}
		}
		return interfaces;
	}

	//gives the Forward Flag given the destination (resturns 2 if the destination is in DTN graph and not in Intra-partition graph)
	int GSTARProtocol::GetForwardFlag(Ipv4Address Addr)
	{
		int flag;		
		Ipv4Address next_hop;
		next_hop = m_state_gstar.getNextHopIP (Addr);
		cout << next_hop << endl;
		if  (next_hop != "0.0.0.0")
		{
			flag =  m_state_gstar.forward_flag (Addr);		
			cout << flag << endl;
					
		}
		else
		{
			next_hop = m_state_gstar.getDTNNextHopIP (Addr);	
			if(next_hop != "0.0.0.0")
				flag = 2;
			else
				flag = 0;		
		}
		cout << "In forward flag: " << flag << endl;
		return flag;
	}

	//updates neighbor probability table
	void GSTARProtocol::UpdateNeighborProbTable (Ipv4Address addr)
	{
		m_state_gstar.UpdateNeighborProbabilityTable ();
		NeighborProbabilitySet curr_neighbor_list = m_state_gstar.NeighborProbabilityTable(); 

		cout << "DTN:Neighbor Table at " << addr << endl;
		for (NeighborProbabilitySet::iterator it = curr_neighbor_list.begin ();it != curr_neighbor_list.end (); it++)
    	        {
                         cout << "IP:" << it->neighborAddr << endl;
			 cout << "on time" << it->on_time << endl;
			 cout << "off time" << it->off_time << endl;
		}
	}
	
	//gives neighbor table (used in sending summary vector i.e starting DTN) 
	NeighborSet GSTARProtocol::GetNeighborTable ()	
	{
		NeighborSet curr_neighbor_list = m_state_gstar.NeighborTable();
		return curr_neighbor_list;
	}
		

	Ptr<Packet> GSTARProtocol::LSA_ACK (const Ipv4Address LSA_addr, uint32_t seq_no, Ipv4Address m_mainAddress)
	{
        	//NS_LOG_FUNCTION(this);
	
		Ptr<Packet> packet = Create<Packet> ();//sends link probe
		mf::MessageHeader header;

		mf::MessageHeader::LSA_ACK &lsa_ack = header.GetLSA_ACK ();
		lsa_ack.src = LSA_addr;

		header.SetMessageType (10);//sets message type = 2 for link probe ACK packet
		header.SetSequenceNumber (seq_no);//sets message sequence number = sequence number in link probe received
        	header.SetMessageAddr(m_mainAddress);//sets message addr = main address
		packet->AddHeader(header);//add header to the packet

		return packet;
	}

	uint32_t GSTARProtocol::GetLSASeqNo ()
	{
		uint32_t seq_no = LSASequenceNumber - 1;
		return seq_no;
	}
	
}



