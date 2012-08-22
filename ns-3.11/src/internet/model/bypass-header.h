/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats

#ifndef BYPASS_HEADER_H
#define BYPASS_HEADER_H

#include <stdint.h>
#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/sequence-number.h"

namespace ns3 
{
	namespace bypass
	{
		enum MessageType
		{
				NONE = 1,
				RTS = 2,
				CTS = 4,
				ACK = 8,
				DATA = 16
		};
		class BypassHeader : public Header
		{
			public:
			
  			BypassHeader ();
  			virtual ~BypassHeader ();

			//sets type of message
  			void SetMessageType (MessageType messageType)
  			{
    				m_messageType = messageType;
  			}

			//gives type of message
			uint32_t GetMessageType () const
  			{
    				return m_messageType;
  			}

			//gives sequence number of link probe packet
			SequenceNumber32 GetSequenceNumber () const
  			{
			    	return m_sequenceNumber;
  			}

			//sets Parent, constructor and attributes (if required)
			//used in both AODV and OLSR codes
			static TypeId GetTypeId (void);
                        virtual TypeId GetInstanceTypeId (void) const;
                        virtual void Print (std::ostream &os) const;
                        virtual uint32_t GetSerializedSize (void) const;
                        virtual void Serialize (Buffer::Iterator start) const;
                        virtual uint32_t Deserialize (Buffer::Iterator start);
			
			private:
			Ipv4Address			m_originalSource;
			Ipv4Address			m_intermediateSource;
			Ipv4Address			m_intermediateDestination;
			Ipv4Address			m_destination;
			uint32_t			m_hopCount;
			SequenceNumber32	m_sequenceNumber;
			MessageType			m_messageType;
			//TODO: add list of interfaces
			//TODO: add data
		};
	}
}

#endif /* BYPASS_HEADER_H */




