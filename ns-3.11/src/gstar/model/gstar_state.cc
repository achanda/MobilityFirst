#include "gstar_state.h"
#include <iostream>
#include "ns3/log.h"

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
          		break;
        	}
    	}
}

//inserts the neighbour information in the neighbor table
void GSTARState::InsertNeighborTuple (const Ipv4Address &Addr, int64_t curr_sett, uint32_t seq_no, Time now)
{
	//first checks if there is already an entry for this node in the neighbor table
  	for (NeighborSet::iterator it = m_neighborSet.begin ();it != m_neighborSet.end (); it++)
    	{
		//if there is an entry
      		if (it->neighborAddr == Addr)
        	{
                    	it->SETT = curr_sett; //updates the current SETT
			it->SETT_array[seq_no] = curr_sett; //updates the SETT array for calculating the LETT
			it->neighbor_expiration_time = now + GSTAR_NEIGHBOR_TIMER; //resechdule the timer
          		return;
        	}
   	}
	
	//if there isn't an entry
	NeighborTuple tuple; // create a new entry
	tuple.neighborAddr = Addr; //address of the neigbhor
	tuple.SETT = curr_sett; //current SETT
	tuple.neighbor_expiration_time = now + GSTAR_NEIGHBOR_TIMER; //sechdule the timer
	
	//initilize the SETT array 
	for(int i=0; i<10; i++)
		tuple.SETT_array[i] = 0;

	tuple.SETT_array[seq_no] = curr_sett; //insert the SETT in the array

  	m_neighborSet.push_back (tuple); //add the entry in the neighbor table
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
		tent_entry.sett_sum = it->SETT; // SETT of the neighbor

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

		tent_entry.lett_sum = lett; // LETT of the neighbor

		tent_entry.nexthopAddr = it->neighborAddr; // next-hop address = neighbor address
		tent_entry.path[0] = it->neighborAddr; // path entry
		tent_entry.path_length = 1; // path length
		
		m_tentative_table.push_back (tent_entry); // inserts tentative entry
	}
		
	Ipv4Address destination_ip;
	Ipv4Address next_hop_ip;
	Ipv4Address path_ip[50];
	int i,m;
	int64_t min_wt = 4294967295;
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
	
		int skip1 = 0;
		int skip2 = 0;
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
					m_tentative_table.push_back (tent_entry);
			}
		}

		min_wt = 4294967295;
		int size=0;
		for(tentative_table::iterator iter1 = m_tentative_table.begin(); iter1 != m_tentative_table.end(); ++iter1)
			size++;
		if(size==0)
			break;
	}
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
	for(forward_table::iterator iter = m_forward_table.begin(); iter != m_forward_table.end(); ++iter)
	{
		if(destinationIP == iter->destinationAddr)
			return iter->nexthopAddr;
	} 
	return 0;
}

//returns the store/forward decision flag given the address of the destination node
int GSTARState::forward_flag (Ipv4Address destinationIP)
{
	int forward_flag=0;
	int64_t lett_cost=0,sett_cost=0;
	Ipv4Address nextHopIP=0;

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
	
	if(nextHopIP == 0)
		return 0;
        
	// if the SETT of the path is bad as compared to LETT of the path
	// forward decision is made only if next hop SETT is exceptionally good as compared to LETT
	if(1.1 * lett_cost < sett_cost)
	{
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

				if(lett > 1.1 * it->SETT)
					forward_flag = 1;
				break;
			}
		}
	}
	else 
		forward_flag = 1;

	return forward_flag;
}

}



