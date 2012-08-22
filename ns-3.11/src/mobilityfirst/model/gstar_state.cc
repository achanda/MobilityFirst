#include "gstar_state.h"
#include "ns3/log.h"
#include <iostream>
//functions for manipulating neighbor table, LSA table and Forwarding table

using namespace std;

namespace ns3 
{

//finds neighbor information, given its address
NeighborTuple* GSTARState::FindNeighborTuple (Ipv4Address const &Addr)
{
  	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
    	{
      		if (it->neighborAddr == Addr)
        		return &(*it);
    	}
  	return NULL;
}

//removes the information of the neighbor given its address
void GSTARState::EraseNeighborTuple (const Ipv4Address Addr)
{
  	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
    	{
      		if (it->neighborAddr == Addr)
        	{
          		it = m_neighborSet.erase (it);
			cout << " in erase neighbor tuple" << endl;
			//forward_table_computation (myIP);
          		break;
        	}
    	}
}

//inserts the neighbour information in the neighbor table
int GSTARState::InsertNeighborTuple (const Ipv4Address &Addr, int64_t curr_sett, uint32_t seq_no, Time now, Ipv4Address my_IfaceAddr,Ipv4Address sender_IfaceAddr,Ipv4Address m_mainAddress)
{
	int new_neighbor_flag = 0;
	//first checks if there is already an entry for this node in the neighbor table
  	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
    	{
		//if there is an entry
      		if (it->neighborAddr == Addr)
        	{
                    	it->SETT = curr_sett; //updates the current SETT
			it->SETT_array[seq_no] = curr_sett; //updates the SETT array for calculating the LETT
			it->neighbor_expiration_time = now + GSTAR_NEIGHBOR_TIMER; //resechdule the timer
		
			if (it->prev_seq_no != (seq_no + 1))
			{
				for(uint32_t i = (it->prev_seq_no + 1); i<seq_no; i++)
					it->SETT_array[i] = 0;
			}

			it->prev_seq_no = seq_no;
			
			int seq_no_1 = seq_no - 1;
			int seq_no_2 = seq_no - 2;

			if(seq_no == 0)
			{
				seq_no_1 = 9;
				seq_no_2 = 8;
			}

			if(seq_no == 1)
				seq_no_2 = 9;

			int count = 3;
						
			it->SETT_avg = (it->SETT_array[seq_no] + it->SETT_array[seq_no_1] + it->SETT_array[seq_no_2]);
		
			if (it->SETT_array[seq_no_1] == 0)
				count--;

			if (it->SETT_array[seq_no_2] == 0)
				count--;

			//cout << "Count" << count << endl;
			//cout << "Seq 1" << seq_no_1 << endl;
			//cout << "Seq 2" << seq_no_2 << endl;
			it->SETT_avg = it->SETT_avg/count;
			
			////it->LETT = (0.5 * it->SETT_avg) + ((1 - 0.5) * it->LETT);

			int count_lett = 0;
        		int64_t lett = 0;
                	//calculating LETT from past 10 SETTs
                	for (int j=0; j<10; j++)
        		{
                        	if(it->SETT_array[j] != 0)       
           		      	{        
                     	        	lett = lett + it->SETT_array[j];
                      		        count_lett++;
                       		}
                	}
			it->LETT = lett/count_lett;

			new_neighbor_flag = 0;

			forward_table_computation (m_mainAddress);

          		return new_neighbor_flag;
        	}
   	}
	
	//if there isn't an entry
	NeighborTuple tuple; // create a new entry
	tuple.neighborAddr = Addr; //address of the neigbhor
	tuple.SETT = curr_sett; //current SETT
	tuple.neighbor_expiration_time = now + GSTAR_NEIGHBOR_TIMER; //sechdule the timer
	tuple.my_ifac = my_IfaceAddr;
	tuple.neighbor_iface = sender_IfaceAddr;
	tuple.prev_seq_no = seq_no;
	tuple.SETT_avg = curr_sett;
	tuple.LETT = curr_sett;
	
	//initilize the SETT array 
	for(int i=0; i<10; i++)
		tuple.SETT_array[i] = 0;

	tuple.SETT_array[seq_no] = curr_sett; //insert the SETT in the array

  	m_neighborSet.push_back (tuple); //add the entry in the neighbor table
	
	new_neighbor_flag = 1;
	forward_table_computation (m_mainAddress);
	
	return new_neighbor_flag;
}

//returns the neighbor table
NeighborSet GSTARState::NeighborTable ()
{
	return m_neighborSet;
}

//finds LSA of a node given its address
LSATuple* GSTARState::FindLSATuple (Ipv4Address const &Addr)
{
  	for (LSASet::iterator it = m_lsaSet.begin ();it != m_lsaSet.end (); it++)
    	{
      		if (it->Addr == Addr)
        		return &(*it);
    	}
  	return NULL;
}

//removes the LSA of the node given its address
void GSTARState::EraseLSATuple (const Ipv4Address Addr)
{
  	for (LSASet::iterator it = m_lsaSet.begin ();it != m_lsaSet.end (); it++)
    	{
      		if (it->Addr == Addr)
        	{
          		it = m_lsaSet.erase (it);
			cout << " in erase lsa tuple" << endl;
			//forward_table_computation (myIP);
          		break;
        	}
    	}
}

//inserts the LSA in the LSA table
int GSTARState::InsertLSATuple (const Ipv4Address &Addr, int64_t sett[], int64_t lett[], Ipv4Address n_Addr[], uint32_t seq_no, int siz,Time now,const Ipv4Address myIP)
{
	int lsa_flag = 0; //used for determining whether to re-broadcast this LSA message
	
	//first checks if there is already an entry for this node in the LSA table
	for (LSASet::iterator it = m_lsaSet.begin ();it != m_lsaSet.end (); it++)
    	{
		//if there is an entry
      		if (it->Addr == Addr)
        	{
			//checks to see if the sequence number of the saved LSA is greater than the one received
                    	if (it->seq_no >= seq_no)
			{
				//if it is, discards the LSA
				lsa_flag = 0;
          			return lsa_flag;
			}      
			//if it isn't, updates the entry
			else
			{
				it->size_lsa = siz; //total number on neighbors in the LSA 
				it->seq_no = seq_no; //sequence number of teh LSA received
				it->lsa_expiration_time = now + GSTAR_LSA_TIMER; //resechdule the timer
				it->SETT = (int64_t*)malloc(siz*sizeof(int64_t));
            			it->LETT = (int64_t*)malloc(siz*sizeof(int64_t));
         			it->neighbor_Addr = (Ipv4Address*)malloc(siz*sizeof(Ipv4Address));

				for(int i=0; i<siz; i++)
				{
					it->neighbor_Addr[i] = n_Addr[i]; //inserts neigbhor addresses
					it->SETT[i] = sett[i]; //inserts SETTs of the neighbors
					it->LETT[i] = lett[i]; //inserts LETTs of the neighbors
				}
				
				lsa_flag = 1;
				forward_table_computation (myIP);
				return lsa_flag;
			}
				
		}
   	}

	//if there isn't an entry
	LSATuple *tuple = (LSATuple*)malloc(sizeof(LSATuple)); // create a new entry
	tuple->Addr = Addr; //address of the node
	tuple->size_lsa = siz; //total number on neighbors in the LSA 
	tuple->seq_no = seq_no; //sequence number of the LSA received
	tuple->lsa_expiration_time = now + GSTAR_LSA_TIMER; //resechdule the timer

	tuple->SETT = (int64_t*)malloc(siz*sizeof(int64_t));
        tuple->LETT = (int64_t*)malloc(siz*sizeof(int64_t));
        tuple->neighbor_Addr = (Ipv4Address*)malloc(siz*sizeof(Ipv4Address));
	
	for(int i=0; i<siz; i++)
	{
		tuple->neighbor_Addr[i] = n_Addr[i]; //inserts neigbhor addresses
		tuple->SETT[i] = sett[i]; //inserts SETTs of the neighbors
		tuple->LETT[i] = lett[i]; //inserts LETTs of the neighbors
	}
	
	m_lsaSet.push_back (*tuple); //add the entry in the LSA table

	lsa_flag = 1;

	if(lsa_flag == 1)
		forward_table_computation (myIP);
		
	return lsa_flag;
}

//returns the LSA table
LSASet GSTARState::LSATable ()
{
	return m_lsaSet;
}

//Djisktra's algorithm (forward search) 
void GSTARState::forward_table_computation (Ipv4Address myIP)
{
	int pl;
        int size_neighbor_table = 0;
	Ipv4Address confirmed[50];	
	int size_confirmed=0;

	m_forward_table.clear();
	//checks to see if the neighbor field is empty
	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
	{
		size_neighbor_table++;
	}

	//returns if the neighbor table is empty
	if(size_neighbor_table == 0)
		return;

	//initilizes the Tentative table with the neighbors
	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
    	{
		struct tentative tent_entry;
		
		tent_entry.destinationAddr = it->neighborAddr;// destination address = neighbor address
		tent_entry.sett_sum = it->SETT_avg; // SETT of the neighbor

		//int count = 0;
                //int64_t lett = 0;
                //calculating LETT from past 10 SETTs
                //for (int j=0; j<10; j++)
                //{
                  //      if(it->SETT_array[j] != 0)       
                   //     {        
                     //           lett = lett + it->SETT_array[j];
                      //          count++;
                       // }
                //}

		//lett = (0.8 * it->SETT) + ((1 - 0.8) * ((lett-it->SETT)/(count - 1)));

		tent_entry.lett_sum = it->LETT; // LETT of the neighbor

		tent_entry.nexthopAddr = it->neighborAddr; // next-hop address = neighbor address
		tent_entry.path[0] = it->neighborAddr; // path entry
		tent_entry.path_length = 1; // path length
		//cout << "inserted " << tent_entry.destinationAddr << endl;
		m_tentative_table.push_back (tent_entry); // inserts tentative entry
	}
		
	Ipv4Address destination_ip;
	Ipv4Address next_hop_ip;
	Ipv4Address path_ip[50];
	int i,m;
	int64_t min_wt = 4294967295u;
   	int64_t sett_curr,lett_curr;

	while(1)
	{
		// finds the least cost (SETT) destination from the tentative list
		for(tentative_table::iterator iter = m_tentative_table.begin(); iter != m_tentative_table.end(); ++iter)
		{
				sett_curr = iter->sett_sum;
				if(min_wt > sett_curr)
				{
					min_wt = sett_curr;
					lett_curr = iter->lett_sum;
					destination_ip = iter->destinationAddr;
					next_hop_ip = iter->nexthopAddr;
					pl = iter->path_length;
					for (m=0;m<pl;m++)
						path_ip[m] = iter->path[m];
				}
		}
	
		//remove that entry from the tentative list
		EraseTentativeEntry (destination_ip);
	
		int skip1;
		int skip2;
		struct forward entry;
	
		entry.destinationAddr = destination_ip;
		entry.sett_sum = min_wt;
		entry.lett_sum = lett_curr;
		entry.nexthopAddr = next_hop_ip;
		entry.path_length = pl;
		for (i=0;i<pl;i++)
			entry.path[i] = path_ip[i];

		// inserts that entry to forward table 
		m_forward_table.push_back (entry);

		confirmed[size_confirmed] = destination_ip;
		size_confirmed++;

		// finds the LSA of the destination inserted in the forward table
		LSATuple* lsa_curr = FindLSATuple (destination_ip);

		if(lsa_curr)
		{
			// insert the nodes from the LSA as the destination in the tentative table
			int length = lsa_curr->size_lsa;
			for(i=0;i<length;i++)
			{
				skip1 = 0;
				skip2 = 0;
				struct tentative tent_entry;
				tent_entry.sett_sum = lsa_curr->SETT[i] + min_wt;
				tent_entry.lett_sum = lsa_curr->LETT[i] + lett_curr;
				tent_entry.nexthopAddr = next_hop_ip;
				tent_entry.destinationAddr = lsa_curr->neighbor_Addr[i];
				tent_entry.path_length = pl+1;
				for(m=0;m<pl;m++)
					tent_entry.path[m+1] = path_ip[m];
				tent_entry.path[0] = lsa_curr->neighbor_Addr[i];
	
				//checks if this destination already exists in the forward table
				for(int j=0;j<size_confirmed;j++)
				{
					if(lsa_curr->neighbor_Addr[i] == confirmed[j])
						skip1 = 1;
				}
	
				// checks if there is already an entry for this destination in the tentative table
				// if there is, the entry with least SETT is selected
				for(tentative_table::iterator iter0 = m_tentative_table.begin(); iter0 != m_tentative_table.end(); ++iter0)
				{
					if(tent_entry.destinationAddr == iter0->destinationAddr)
					{
						if(tent_entry.sett_sum > iter0->sett_sum)
							skip2 = 1;
						else
						{
							EraseTentativeEntry (tent_entry.destinationAddr);
							break;
						}
					}
				}

				if ( skip1 == 0 && skip2 == 0 && tent_entry.destinationAddr != myIP)
				{
					m_tentative_table.push_back (tent_entry);
					//cout << "inserted " << tent_entry.destinationAddr << endl;
				}
			}
		}

		min_wt = 4294967295u;
		int size=0;
		for(tentative_table::iterator iter1 = m_tentative_table.begin(); iter1 != m_tentative_table.end(); ++iter1)
			size++;
		if(size==0)
			break;
	}
        //int flag;
	//cout << "Forward Table at node " << myIP << endl;
	//for(forward_table::iterator iter1 = m_forward_table.begin(); iter1 != m_forward_table.end(); ++iter1)
	//{
	//		cout << "destination" << iter1->destinationAddr << endl;
	//		cout << "sett" << iter1->sett_sum << endl;
	//		cout << "lett" << iter1->lett_sum << endl;
	//		cout << "next hop" << iter1->nexthopAddr << endl;
   	//		cout << "path length" << iter1->path_length << endl;
	//		pl=iter1->path_length;
	//		cout << "path" << endl;
          //      	for(int d = pl-1 ; d >=0 ; d--)
		//		cout << iter1->path[d] << endl;
		//	int flag =  forward_flag(iter1->destinationAddr);
		//	cout << "Flag " << flag << endl;
	//}

}

//removes the entry of the destination node from the tenative table given its address		
void GSTARState::EraseTentativeEntry (const Ipv4Address Addr)
{
	for(tentative_table::iterator it = m_tentative_table.begin(); it != m_tentative_table.end(); ++it)
      	{
      		if (it->destinationAddr == Addr)
        	{
          		it = m_tentative_table.erase (it);
          		break;
        	}
    	}
}

//returns the Forward table
forward_table GSTARState::ForwardTable ()
{
	return m_forward_table;
}

//returns the address of the next hop node (neighbor) given the address of destination node
Ipv4Address GSTARState::getNextHopIP (Ipv4Address destinationIP)
{
	cout << " in getNextHopIP " << endl;
	for(forward_table::iterator iter = m_forward_table.begin(); iter != m_forward_table.end(); ++iter)
	{
		if(destinationIP == iter->destinationAddr)
			return iter->nexthopAddr;
	} 
	return "0.0.0.0";
}

//returns the store/forward decision flag given the address of the destination node
int GSTARState::forward_flag (Ipv4Address destinationIP)
{
	int forward_flag=0;
	int64_t lett_cost=0,sett_cost=0;
	Ipv4Address nextHopIP = "0.0.0.0";
	cout << " in forward flag " << endl;
	// get the address of the next hop (neighbor) node & SETT and LETT of the path to the destination
	for(forward_table::iterator iter = m_forward_table.begin(); iter != m_forward_table.end(); ++iter)
	{
		if(destinationIP == iter->destinationAddr)
		{
			nextHopIP = iter->nexthopAddr;
   			lett_cost = iter->lett_sum;
			sett_cost = iter->sett_sum;
		}
	}
	
	if(nextHopIP == "0.0.0.0")
		return 0;
        
	// if the SETT of the path is bad as compared to LETT of the path
	// forward decision is made only if next hop SETT is exceptionally good as compared to LETT
	if(sett_cost > 1.1 * lett_cost)
	//if((lett_cost) < sett_cost)
	{
		cout << "in if" << endl;
		for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
    		{
			if(nextHopIP == it->neighborAddr)
			{
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

				//if(lett > 1.1 * it->SETT)
				//	forward_flag = 1;
				break;
			}
		}
	}
	else 
		forward_flag = 1;

	return forward_flag;
}

//checks to see if a DLSA exists
bool GSTARState::FindDLSA(Ipv4Address const &Addr, int64_t const &seq_no)
{
  	for (DLSASet::iterator it = m_dlsaSet.begin ();it != m_dlsaSet.end (); it++)
    	{
      		if (it->nodeAddr == Addr)
		{
			if(it->seq_no < seq_no)
	        		return false;
			else
				return true;
		}
    	}
  	return false;
}

//finds DLSA of a node given its address
DLSATuple* GSTARState::FindDLSATuple (Ipv4Address const &Addr)
{
  	for (DLSASet::iterator it = m_dlsaSet.begin ();it != m_dlsaSet.end (); it++)
    	{
      		if (it->nodeAddr == Addr)
        		return &(*it);
    	}
  	return NULL;
}

//removes the DLSA of the node given its address
void GSTARState::EraseDLSATuple (const Ipv4Address Addr)
{
	//cout << "in EraseDLSATuple for " << Addr << endl;
  	for (DLSASet::iterator it = m_dlsaSet.begin ();it != m_dlsaSet.end (); it++)
    	{
      		if (it->nodeAddr == Addr)
        	{
          		it = m_dlsaSet.erase (it);
          		break;
        	}
    	}
}

//inserts the LSA in the LSA table
int GSTARState::InsertDLSATuple (const Ipv4Address &Addr, int64_t prob[], Ipv4Address n_Addr[], uint32_t seq_no, int siz,Time now,const Ipv4Address myIP)
{
	int dlsa_flag = 0; //used for determining whether to re-broadcast this DLSA message
	//first checks if there is already an entry for this node in the LSA table
	for (DLSASet::iterator it = m_dlsaSet.begin ();it != m_dlsaSet.end (); it++)
    	{
		//if there is an entry
      		if (it->nodeAddr == Addr)
        	{
			//checks to see if the sequence number of the saved DLSA is greater than the one received
                    	if (it->seq_no >= seq_no)
			{
				//if it is, discards the DLSA
				dlsa_flag = 0;
				return dlsa_flag;
			}      
			//if it isn't, updates the entry
			else
			{
				it->size_dlsa = siz; //total number on nodes in the DLSA 
				it->seq_no = seq_no; //sequence number of the DLSA received
				it->dlsa_expiration_time = now + GSTAR_DLSA_TIMER; //resechdule the timer
				it->probability = (int64_t*)malloc(siz*sizeof(int64_t));
            			it->Addr = (Ipv4Address*)malloc(siz*sizeof(Ipv4Address));

				for(int i=0; i<siz; i++)
				{
					it->Addr[i] = n_Addr[i]; //inserts node addresses
					it->probability[i] = prob[i]; //inserts probabilities of the nodes
				}

				dlsa_flag = 1;
				dtn_forward_table_computation (myIP);
				return dlsa_flag;
			}
				
		}
   	}

	//if there isn't an entry
	DLSATuple *tuple = (DLSATuple*)malloc(sizeof(DLSATuple)); // create a new entry
	tuple->nodeAddr = Addr; //address of the node
	tuple->size_dlsa = siz; //total number on neighbors in the LSA 
	tuple->seq_no = seq_no; //sequence number of the LSA received
	tuple->dlsa_expiration_time = now + GSTAR_DLSA_TIMER; //resechdule the timer

	tuple->probability = (int64_t*)malloc(siz*sizeof(int64_t));
       	tuple->Addr = (Ipv4Address*)malloc(siz*sizeof(Ipv4Address));
	
	for(int i=0; i<siz; i++)
	{
		tuple->Addr[i] = n_Addr[i]; //inserts node addresses
		tuple->probability[i] = prob[i]; //inserts probabilities of the nodes
	}
	
	m_dlsaSet.push_back (*tuple); //add the entry in the LSA table

	dlsa_flag = 1;

	if(dlsa_flag == 1)
		dtn_forward_table_computation (myIP);
		
	return dlsa_flag;
}
//returns the DLSA table
DLSASet GSTARState::DLSATable ()
{
	return m_dlsaSet;
}

//Djisktra's algorithm (forward search) 
void GSTARState::dtn_forward_table_computation (Ipv4Address myIP)
{
	int pl;
        int size_neighbor_table = 0;
	Ipv4Address confirmed[50];	
	int size_confirmed=0;

	m_dtn_forward_table.clear();
	//checks to see if the neighbor field is empty
	for (NeighborProbabilitySet::iterator it = m_neighborProbabilitySet.begin ();it != m_neighborProbabilitySet.end (); it++)
	{
		size_neighbor_table++;
	}

	//returns if the neighbor table is empty
	if(size_neighbor_table == 0)
		return;

	//initilizes the Tentative table with the neighbors
	for (NeighborProbabilitySet::iterator it = m_neighborProbabilitySet.begin ();it != m_neighborProbabilitySet.end (); it++)
    	{
		struct dtn_tentative tent_entry;
		
		tent_entry.destinationAddr = it->neighborAddr;// destination address = neighbor address
		tent_entry.probability = (it->on_time * 100)/(it->on_time + it->off_time) ; // probability of the neighbor
		tent_entry.probability = 100 - tent_entry.probability; 
		tent_entry.nexthopAddr = it->neighborAddr; // next-hop address = neighbor address
		tent_entry.path[0] = it->neighborAddr; // path entry
		tent_entry.path_length = 1; // path length
		//cout << "inserted " << tent_entry.destinationAddr << endl;
		m_dtn_tentative_table.push_back (tent_entry); // inserts tentative entry
	}
		
	Ipv4Address destination_ip;
	Ipv4Address next_hop_ip;
	Ipv4Address path_ip[50];
	int i,m;
	int64_t min_wt = 4294967295u;
   	int64_t prob_curr;

	while(1)
	{
		// finds the least cost destination from the tentative list
		for(dtn_tentative_table::iterator iter = m_dtn_tentative_table.begin(); iter != m_dtn_tentative_table.end(); ++iter)
		{
				prob_curr = iter->probability;
				if(min_wt > prob_curr)
				{
					min_wt = prob_curr;
					destination_ip = iter->destinationAddr;
					next_hop_ip = iter->nexthopAddr;
					pl = iter->path_length;
					for (m=0;m<pl;m++)
						path_ip[m] = iter->path[m];
				}
		}
	
		//remove that entry from the tentative list
		EraseDTNTentativeEntry (destination_ip);
	
		int skip1;
		int skip2;
		struct dtn_forward entry;
	
		entry.destinationAddr = destination_ip;
		entry.probability = min_wt;
		entry.nexthopAddr = next_hop_ip;
		entry.path_length = pl;
		for (i=0;i<pl;i++)
			entry.path[i] = path_ip[i];

		// inserts that entry to forward table 
		m_dtn_forward_table.push_back (entry);

		confirmed[size_confirmed] = destination_ip;
		size_confirmed++;

		// finds the LSA of the destination inserted in the forward table
		DLSATuple* dlsa_curr = FindDLSATuple (destination_ip);

		if(dlsa_curr)
		{
			// insert the nodes from the LSA as the destination in the tentative table
			int length = dlsa_curr->size_dlsa;
			for(i=0;i<length;i++)
			{
				skip1 = 0;
				skip2 = 0;
				struct dtn_tentative tent_entry;
				tent_entry.probability = (100 - dlsa_curr->probability[i]) + min_wt;
				tent_entry.nexthopAddr = next_hop_ip;
				tent_entry.destinationAddr = dlsa_curr->Addr[i];
				tent_entry.path_length = pl+1;
				for(m=0;m<pl;m++)
					tent_entry.path[m+1] = path_ip[m];
				tent_entry.path[0] = dlsa_curr->Addr[i];
	
				//checks if this destination already exists in the forward table
				for(int j=0;j<size_confirmed;j++)
				{
					if(dlsa_curr->Addr[i] == confirmed[j])
						skip1 = 1;
				}
	
				// checks if there is already an entry for this destination in the tentative table
				// if there is, the entry with least SETT is selected
				for(dtn_tentative_table::iterator iter0 = m_dtn_tentative_table.begin(); iter0 != m_dtn_tentative_table.end(); ++iter0)
				{
					if(tent_entry.destinationAddr == iter0->destinationAddr)
					{
						if(tent_entry.probability > (1 -iter0->probability))
							skip2 = 1;
						else
						{
							EraseDTNTentativeEntry (tent_entry.destinationAddr);
							break;
						}
					}
				}

				if ( skip1 == 0 && skip2 == 0 && tent_entry.destinationAddr != myIP)
				{
					m_dtn_tentative_table.push_back (tent_entry);
					//cout << "inserted " << tent_entry.destinationAddr << endl;
				}
			}
		}

		min_wt = 4294967295u;
		int size=0;
		for(dtn_tentative_table::iterator iter1 = m_dtn_tentative_table.begin(); iter1 != m_dtn_tentative_table.end(); ++iter1)
			size++;
		if(size==0)
			break;
	}
}

//removes the entry of the destination node from the DTN tenative table given its address		
void GSTARState::EraseDTNTentativeEntry (const Ipv4Address Addr)
{
	for(dtn_tentative_table::iterator it = m_dtn_tentative_table.begin(); it != m_dtn_tentative_table.end(); ++it)
      	{
      		if (it->destinationAddr == Addr)
        	{
          		it = m_dtn_tentative_table.erase (it);
          		break;
        	}
    	}
}

//returns the DTN Forward table
dtn_forward_table GSTARState::DTNForwardTable ()
{
	return m_dtn_forward_table;
}

//returns the address of the next hop node (neighbor) from DTN table given the address of destination node
Ipv4Address GSTARState::getDTNNextHopIP (Ipv4Address destinationIP)
{
	cout << " in getDTNNextHopIP " << endl;
	for(dtn_forward_table::iterator iter = m_dtn_forward_table.begin(); iter != m_dtn_forward_table.end(); ++iter)
	{
		if(destinationIP == iter->destinationAddr)
			return iter->nexthopAddr;
	} 
	return "0.0.0.0";
}

//finds neighbor probabilty information, given its address
NeighborProbabilityTuple* GSTARState::FindNeighborProbabilityTuple (Ipv4Address const &Addr)
{
  	for(NeighborProbabilitySet::iterator it = m_neighborProbabilitySet.begin(); it !=m_neighborProbabilitySet.end(); ++it)
    	{
      		if (it->neighborAddr == Addr)
        		return &(*it);
    	}
  	return NULL;
}

//updates the neighbor probabilty table
void GSTARState::UpdateNeighborProbabilityTable ()
{
	//if the node in the neighbor probability table still exists in the neighbor table, increment on-time, else, increment off time
	for(NeighborProbabilitySet::iterator it = m_neighborProbabilitySet.begin(); it !=m_neighborProbabilitySet.end(); ++it)
      	{
		if (FindNeighborTuple (it->neighborAddr) != NULL)
			it->on_time = it->on_time + 1;
		else
			it->off_time = it->off_time + 1;
	}
	
	// creates new entry for the neighbors not in the neighbor probability table
	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
	{
		if (FindNeighborProbabilityTuple (it->neighborAddr) == NULL)
		{
			NeighborProbabilityTuple tuple; // create a new entry
			tuple.neighborAddr = it->neighborAddr; //address of the neigbhor
			tuple.on_time = 1; // on time initilized to 1
			tuple.off_time = 0; // off time initilized to 0
			tuple.neighbor_iface = it->neighbor_iface;
			tuple.my_ifac = it->my_ifac;
			m_neighborProbabilitySet.push_back (tuple); //add the entry in the neighbor probability table

		}
	}
}

//returns the neighbor probabilty table
NeighborProbabilitySet GSTARState::NeighborProbabilityTable ()
{
	return m_neighborProbabilitySet;
}

}



