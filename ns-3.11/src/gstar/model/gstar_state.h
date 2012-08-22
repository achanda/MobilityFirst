#ifndef __GSTAR_STATE_H__
#define __GSTAR_STATE_H__

//functions for manipulating neighbor table, LSA table and Forwarding table

#include "gstar_repositories.h"

#define GSTAR_NEIGHBOR_TIMER   Seconds (10)
#define GSTAR_LSA_TIMER   Seconds (10)

namespace ns3 {

using namespace gstar;

class GSTARState
{
	protected:
		NeighborSet m_neighborSet; //Neighbor Table
		LSASet m_lsaSet; //LSA Table
		tentative_table m_tentative_table; //Tentative Table
		forward_table m_forward_table; //Forward Table
		

	public:
		GSTARState () {}

		NeighborTuple* FindNeighborTuple (const Ipv4Address &Addr); //finds neighbor information, given its address

		void EraseNeighborTuple (const Ipv4Address Addr); //removes the information of the neighbor given its address
		//inserts the neighbour information in the neighbor table	
		void InsertNeighborTuple (const Ipv4Address &Addr, int64_t curr_sett, uint32_t seq_no, Time now); 
		NeighborSet NeighborTable (); //returns the neighbor table	

		LSATuple* FindLSATuple (const Ipv4Address &Addr); //finds LSA of a node given its address
		void EraseLSATuple (const Ipv4Address Addr); //removes the LSA of the node given its address
		//inserts the LSA in the LSA table
		int InsertLSATuple (const Ipv4Address &Addr, int64_t sett[], int64_t lett[], Ipv4Address n_Addr[], uint32_t seq_no, int siz,Time now,const Ipv4Address myIP);
		LSASet LSATable (); //returns the LSA table

		void forward_table_computation (Ipv4Address myIP); //constructs a forwarding table from the LSAs received
		void EraseTentativeEntry (const Ipv4Address Addr); //removes the entry of the destination node from the tenative table given its address
		forward_table ForwardTable (); //returns the Forward table

		//returns the address of the next hop node (neighbor) given the address of destination node
		Ipv4Address getNextHopIP (Ipv4Address destinationIP);  	

		//returns the store/forward decision flag given the address of the destination node 					
		int forward_flag (Ipv4Address destinationIP);
};

} 

#endif


		
