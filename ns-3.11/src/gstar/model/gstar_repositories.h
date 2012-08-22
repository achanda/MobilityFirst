#ifndef __GSTAR_REPOSITORIES_H__
#define __GSTAR_REPOSITORIES_H__

//databases for saving neighbor information, LSAs and Forwarding Table

#include <set>
#include <vector>

#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

namespace ns3 
{ 

namespace gstar
{
	//each neighbor entry consists of:
	struct NeighborTuple
	{
		Ipv4Address neighborAddr; //address of neighbor
		int64_t SETT; //current link quality
		int64_t SETT_array[10]; //past 10 link qualities, used in calculating LETT
		Time neighbor_expiration_time; //Time when this neighbor is to be deleted
	};

	typedef std::vector<NeighborTuple> NeighborSet; //a vector contating all the neighbor entries = Neighbor Table

	//each LSA entry consists of:
	struct LSATuple
	{
		Ipv4Address Addr; //address of the node 
		int size_lsa; //total number of neighbors
		uint32_t seq_no; //sequence number of the LSA
		Ipv4Address *neighbor_Addr; //list of all the neighbors of this node
		int64_t *SETT; //list of all the SETTs of the neighbors of this node
		int64_t *LETT; //list of all the LETTs of the neighbors of this node
		Time lsa_expiration_time; //Time when this LSA is to be deleted
	};

	typedef std::vector<LSATuple> LSASet; //a vector contating all the LSA entries = LSA Table

	//each forward table entry consists of:
	struct forward
	{
		Ipv4Address destinationAddr; //address of the destination node 
		int64_t sett_sum; //SETT of the complete path
		int64_t lett_sum; //LETT of the complete path
		Ipv4Address nexthopAddr; //address of the next hop (neighbor) node
		Ipv4Address path[50]; //address of the nodes in the path
		int path_length; //hop count of the complete path
	};

	typedef std::vector<forward> forward_table; //Forward Table

	//Tentative Table is used in Djisktra's (Forward search) algorithm
	//each tentative table entry consists of:
	struct tentative
	{
		Ipv4Address destinationAddr; //address of the destination node 
		int64_t sett_sum; //SETT of the complete path
		int64_t lett_sum; //LETT of the complete path
		Ipv4Address nexthopAddr; //address of the next hop (neighbor) node
		Ipv4Address path[50]; //address of the nodes in the path
		int path_length; //hop count of the complete path
	};

	typedef std::vector<tentative> tentative_table; //Tentative Table
}

}

#endif  /* __GSTAR_REPOSITORIES_H__ */


	




