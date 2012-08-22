#ifndef __HOP_REPOSITORIES_H__
#define __HOP_REPOSITORIES_H__

//databases for HOP

#include <set>
#include <vector>

#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "mf_header.h"
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


#define  CHK_PKT_COUNT 25

namespace ns3 
{ 

namespace mf
{
	//each Data Tuple consists of
	struct DataTuple
	{
		uint32_t chunck_id; //chunck ID
		//int packet_count;
		Ipv4Address source; //source address
		std::vector< Ptr<Packet> > data_pkts; //data packets for that chunck
	};

	typedef std::vector<DataTuple> DataSet; // a vector containing all the data tuples

	//each Bitmap Tuple consists of
	struct BitmapTuple
	{
		uint32_t chunck_id; // chunck ID
		Ipv4Address source; //source address
		char bitmap[CHK_PKT_COUNT]; // bitmap for that chunck
	};

	typedef std::vector<BitmapTuple> BitmapSet; // a vector containing all the Bitmap tuples  

	//each CSYNC Tuple consists of
	struct CSYNCTuple
	{
		int flag;
		EventId id;
		Time csync_expiration_time;
		Ipv4Address source; // source address
		Ipv4Address destination; //destination IP address
		Ptr<Packet> csync_pkt; //CSYNC packet
		Ptr<Packet> csync_ack_pkt; //CSYNC ACK packet
	};

	typedef std::vector<CSYNCTuple> CSYNCSet; //a vector containing all the CSYNC tuple (used in resending) 

	//each Saved Data Tuple consists of
	struct DataTupleSaved
	{
		Ipv4Address source; //source address
		Ipv4Address destination; //destination address
		EventId id;
		Time expiration_time;
		std::vector< Ptr<Packet> > data_pkts; //data packets for that chunck
	};

	typedef std::vector<DataTupleSaved> DataSavedSet; // a vector containing all the saved data tuples (used in resending)

	//each Packet Count Tuple consists of
	struct PacketCountTuple
	{
		uint32_t chunck_id; //chunck ID
		int packet_count; //packet count
		Ipv4Address source; //source address
	};

	typedef std::vector<PacketCountTuple> PktCountSet; // a vector containing all the Packet Count tuples

}

}

#endif  /* _HOP_REPOSITORIES_H__ */
