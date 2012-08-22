/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats
//GSTAR packet format: TYPE,SOURCE ADDRESS,SEQUENCE NUMBER
//LSA packet has an addition to the above
//...LENGTH(say N),NEIGHBOR-1 ADDR,NEIGHBOR-1 SETT,NEIGHBOR-1 LETT,...,NEIGHBOR-N ADDR,NEIGHBOR-N SETT,NEIGHBOR-N LETT

#ifndef BYPASS_HEADER_H
#define BYPASS_HEADER_H

#include <stdint.h>
#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#define BUFFER_SIZE 1000

namespace ns3 
{
	class MessageHeader : public Header
	{
		public:

			MessageHeader ();
			virtual ~MessageHeader ();
		
			void SetDestination (Ipv4Address dest)
			{
				m_destination = dest;
			}
		
			Ipv4Address GetDestination (void)
			{
				return m_destination;
			}
		
			void SetBD (Ipv4Address left)
			{
				m_BD = left;
			}
		
			Ipv4Address GetBD (void)
			{
				return m_BD;
			}
		
			void SetBS (Ipv4Address right)
			{
				m_BS = right;
			}
		
			Ipv4Address GetBS (void)
			{
				return m_BS;
			}
		
			uint32_t GetType (void)
			{
				return m_type;
			}
		
			void SetType (uint32_t type)
			{
				m_type = type;
			}
		
			uint32_t GetSequence(void)
			{
				return m_seq;
			}
		
			void SetSequence (uint32_t s)
			{
				m_seq = s;
			}
		
			void SetBuffer (char *s)
			{
				for (int i=0;i < BUFFER_SIZE; i++)
					buffer[i] = s[i];
			}

			//sets Parent, constructor and attributes (if required)
			//used in both AODV and OLSR codes
			static TypeId GetTypeId (void);
			virtual TypeId GetInstanceTypeId (void) const;
			virtual void Print (std::ostream &os) const;
			virtual uint32_t GetSerializedSize (void) const; //gives the size ok GSTAR packet header
			virtual void Serialize (Buffer::Iterator start) const; //serializes GSTAR packet header
			virtual uint32_t Deserialize (Buffer::Iterator start); //deserializes GSTAR packet header

		private:
		Ipv4Address m_BD; // bd
		Ipv4Address m_BS; // bs
		Ipv4Address m_destination;	//destination address
		uint32_t m_type;			//identifies the packet type
		uint32_t m_seq;
		char buffer[BUFFER_SIZE];
	};
}
#endif /* MF_HEADER_H */
