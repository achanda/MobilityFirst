#ifndef __GSTAR_PROTOCOL_H__
#define __GSTAR_PROTOCOL_H__

//functions for handling GSTAR-Intrapartition packets and tables

#include "mf_header.h"
#include "gstar_state.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/event-garbage-collector.h"
#include "ns3/timer.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h" 
#include "ns3/yans-wifi-phy.h"

namespace ns3 {

using namespace mf;

class GSTARProtocol
{

	public:
		GSTARProtocol ();
		~GSTARProtocol () {}
		Ptr<Packet> SendLP (Ipv4Address m_mainAddress, uint8_t mac_addr[]);//creates link probe packet
		int ProcessLP (mf::MessageHeader lpPacket, Ptr<WifiNetDevice> m_wifi, Ipv4Address my_IfaceAddr, Ipv4Address sender_IfaceAddr, Ipv4Address m_mainAddress);
		//creates link probe ACK packet		
		Ptr<Packet> SendLPACK (const Ipv4Address receiverIface,const Ipv4Address senderIface, uint32_t seq_no, Ipv4Address m_mainAddress);
		//process link probe ACK and forms neighbor table
		int ProcessLPACK (const Ipv4Address neighbor_add, uint32_t seq_no, Ipv4Address m_mainAddress,Ipv4Address my_IfaceAddr, Ipv4Address sender_IfaceAddr);
		Ptr<Packet> SendLSA (Ipv4Address m_mainAddress);//creates LSA packet
		//process LSA , saves it (if new) and forms LSA table
		int ProcessLSA (mf::MessageHeader lsaPacket,Ipv4Address addr, uint32_t seq_no, Ipv4Address m_mainAddress);
		Ptr<Packet> GetPkt();//gives lsa pkt to be re-broadcasted
		Ptr<Packet> SendSummaryVector(Ipv4Address m_mainAddress);//creates summary vector packet
		//creates summary vector ACK packet
		Ptr<Packet> SendSummaryVectorACK(mf::MessageHeader summ_vec_pkt, uint32_t seq_no, Ipv4Address m_mainAddress);
		Ptr<Packet> SendDLSA(Ipv4Address Addr, Ipv4Address m_mainAddress); ////creates DLSA packet
		Ptr<Packet> SendMyDLSA(Ipv4Address m_mainAddress);
		//process DLSA and forms DLSA table
		int ProcessDLSA (mf::MessageHeader dlsaPacket,Ipv4Address addr, uint32_t seq_no, Ipv4Address m_mainAddress);
		Ptr<Packet> LSA_ACK (const Ipv4Address LSA_addr, uint32_t seq_no, Ipv4Address m_mainAddress);
		uint32_t GetLSASeqNo ();

		InterfaceTuple GetInterfaces(Ipv4Address Addr);// gives sender and receiver interfaces given the destination address
		int GetForwardFlag(Ipv4Address Addr);//gives the Forward Flag given the destination (resturns 2 if the destination is in DTN graph and not in Intra-partition graph)
		void UpdateNeighborProbTable (Ipv4Address);//updates neighbor probability table  
		NeighborSet GetNeighborTable ();//gives neighbor table (used in sending summary vector i.e starting DTN) 
		
	private:
		GSTARState m_state_gstar; //for accessing database manipulation (gstar_state.cc)
		EventGarbageCollector m_events_gstar; //used for scheduling NeighborTupleTimerExpire and LSATupleTimerExpire (erasing expired entries)
		uint32_t LPSequenceNumber; //sequence number for link probe packets
		uint32_t LSASequenceNumber; //sequence number for LSA packets
		uint32_t SummVecSequenceNumber; //sequence number for summary vector packets
		uint32_t DLSASequenceNumber; //sequence number for my DLSA packets
		Time link_probe_history[10]; //saves time at which link probe was send
		Ptr<Packet> rebroadcast_lsa_pkt;

		void NeighborTupleTimerExpire (Ipv4Address neighbor_add);//function called when timer for a neighbor entry expires
		void LSATupleTimerExpire (Ipv4Address addr);//function called when timer for a LSA entry expires
		void DLSATupleTimerExpire (Ipv4Address addr);//function called when timer for a DLSA entry expires
                
		

};

} 

#endif

