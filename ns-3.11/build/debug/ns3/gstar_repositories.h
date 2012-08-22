#ifndef __GSTAR_REPOSITORIES_H__
#define __GSTAR_REPOSITORIES_H__

//databases for saving neighbor information, LSAs and Forwarding Table

#include <set>
#include <vector>

#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"

namespace ns3 
{ 

namespace mf
{
	//each neighbor entry consists of:
	struct NeighborTuple
	{
		Ipv4Address neighborAddr; //address of neighbor
		Ipv4Address my_ifac; // my interface address
		Ipv4Address neighbor_iface; // neighbor interface address
		int64_t  SETT; //current link quality
		int64_t  SETT_avg;
		int64_t  LETT;
		uint32_t prev_seq_no;
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

	//each DLSA entry consists of:
	struct DLSATuple
	{
		Ipv4Address nodeAddr; //address of the node 
		int size_dlsa; //total number of neighbors
		uint32_t seq_no; //sequence number of the LSA
		Ipv4Address *Addr; //list of all the nodes in this DLSA Tuple
		int64_t *probability; //list of all the probabilities of the nodes in this DLSA Tuple
		Time dlsa_expiration_time; //Time when this LSA is to be deleted
	};

	typedef std::vector<DLSATuple> DLSASet; //a vector contating all the LSA entries = LSA Table

	//each DTN forward table entry consists of:
	struct dtn_forward
	{
		Ipv4Address destinationAddr; //address of the destination node 
		int64_t probability; //cost of the complete path
		Ipv4Address nexthopAddr; //address of the next hop (neighbor) node
		Ipv4Address path[50]; //address of the nodes in the path
		int path_length; //hop count of the complete path
	};

	typedef std::vector<dtn_forward> dtn_forward_table; //Forward Table

	//Tentative Table is used in Djisktra's (Forward search) algorithm
	//each tentative table entry consists of:
	struct dtn_tentative
	{
		Ipv4Address destinationAddr; //address of the destination node 
		int64_t probability; //cost of the complete path
		Ipv4Address nexthopAddr; //address of the next hop (neighbor) node
		Ipv4Address path[50]; //address of the nodes in the path
		int path_length; //hop count of the complete path
	};

	typedef std::vector<dtn_tentative> dtn_tentative_table; //Tentative Table

	//each Neighbor Probability entry contains
	struct NeighborProbabilityTuple
	{
		Ipv4Address neighborAddr; //address of neighbor
		Ipv4Address neighbor_iface; //address of neighbor
		Ipv4Address my_ifac; // my interface address
		int64_t on_time; //on time
		int64_t off_time; //off time
		int64_t off_time_flag; //off time flag - used to delete the entry
	};

	//a vector contating all the neighbor probability entries = Neighbor Probability Table
	typedef std::vector<NeighborProbabilityTuple> NeighborProbabilitySet;

	//each Interface tuple entry contains
	struct InterfaceTuple
	{                        
		Ipv4Address my_interface;// sender interface                        
		Ipv4Address neighbor_interface; // receiver interface
	};

	struct LSATupleSaved
	{
		Ipv4Address source; //source address
		Ipv4Address destination; //destination address
		uint32_t seq_no;
		EventId id;
		Time expiration_time;
		Ptr<Packet> lsa_pkt; //data packets for that chunck
	};

	typedef std::vector<LSATupleSaved> LSASavedSet;
}

}

#endif  /* __GSTAR_REPOSITORIES_H__ */


	




