/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats
//GSTAR packet format: TYPE,SOURCE ADDRESS,SEQUENCE NUMBER
//LSA packet has an addition to the above
//...LENGTH(say N),NEIGHBOR-1 ADDR,NEIGHBOR-1 SETT,NEIGHBOR-1 LETT,...,NEIGHBOR-N ADDR,NEIGHBOR-N SETT,NEIGHBOR-N LETT

#include "ns3/assert.h"
#include "bypass-header.h"
#include "ns3/log.h"

#define PKT_HEADER_SIZE 20
#define IPV4_ADDRESS_SIZE 4

namespace ns3 
{
	MessageHeader::MessageHeader ()
	{

	}

	MessageHeader::~MessageHeader ()
	{

	}

	//same as used in AODV and OLSR codes
	TypeId MessageHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::MessageHeader")
			.SetParent<Header> ()
			.AddConstructor<MessageHeader> ();
		return tid;
	}

	TypeId MessageHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId ();
	}

	//returns the serialized size of GSTAR packet
	uint32_t MessageHeader::GetSerializedSize (void) const
	{
		uint32_t size = PKT_HEADER_SIZE + BUFFER_SIZE;
		return size;
	}

	void MessageHeader::Print (std::ostream &os) const
	{
		// TODO
	}

	//serializes GSTAR packet
	void MessageHeader::Serialize (Buffer::Iterator start) const
	{
		Buffer::Iterator i = start;
		i.WriteHtonU32 (m_BD.Get ()); //message source address
		i.WriteHtonU32 (m_BS.Get ());
		i.WriteHtonU32 (m_destination.Get ());
		i.WriteHtonU32 (m_type);
		i.WriteHtonU32 (m_seq);
		for(int j=0; j<BUFFER_SIZE; j++)
			i.WriteU8 (buffer[j]);
	}

	//deserializes GSTAR packet
	uint32_t MessageHeader::Deserialize (Buffer::Iterator start)
	{
		uint32_t size;      
		Buffer::Iterator i = start;
		m_BD  = Ipv4Address (i.ReadNtohU32 ());
		m_BS = Ipv4Address (i.ReadNtohU32 ());
		m_destination = Ipv4Address (i.ReadNtohU32 ());
		m_type = i.ReadNtohU32 ();
		m_seq = i.ReadNtohU32 ();
		for(int j=0; j<BUFFER_SIZE; j++)
			buffer[j] = i.ReadU8 ();

		size = PKT_HEADER_SIZE+BUFFER_SIZE; //size = addition of the above sizes

		return size; //returns the size of the GSTAR packet
	}
}
