#include "ns3/assert.h"

#include "bypass-header.h"
#include "ns3/log.h"

#define GSTAR_PKT_HEADER_SIZE 25

namespace ns3 
{

	namespace bypass
	{

		BypassHeader::BypassHeader ()
		{

		}

		BypassHeader::~BypassHeader ()
		{

		}

		//same as used in AODV and OLSR codes
		TypeId BypassHeader::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::bypass::BypassHeader")
				.SetParent<Header> ()
				.AddConstructor<BypassHeader> ();
			return tid;
		}

		TypeId BypassHeader::GetInstanceTypeId (void) const
		{
			return GetTypeId ();
		}

		//returns the total size of the header in bytes
		uint32_t BypassHeader::GetSerializedSize (void) const
		{
			return 4*sizeof(Ipv4Address)+sizeof(uint32_t)+sizeof(SequenceNumber32)+sizeof(MessageType);
		}

		void BypassHeader::Print (std::ostream &os) const
		{
			// TODO
		}

		void BypassHeader::Serialize (Buffer::Iterator start) const
		{
			Buffer::Iterator i = start;
			i.WriteHtonU32 (m_originalSource.Get());
			i.WriteHtonU32 (m_intermediateSource.Get());
			i.WriteHtonU32 (m_intermediateDestination.Get());
			i.WriteHtonU32 (m_destination.Get());
			i.WriteHtonU32 (m_hopCount);
			i.WriteHtonU32 (m_sequenceNumber.GetValue());
			i.WriteHtonU32 (m_messageType);
		}

		uint32_t BypassHeader::Deserialize (Buffer::Iterator start)
		{
			Buffer::Iterator i = start;
			m_originalSource = Ipv4Address(i.ReadNtohU32());
			m_intermediateSource = Ipv4Address(i.ReadNtohU32());
			m_intermediateDestination = Ipv4Address(i.ReadNtohU32());
			m_destination = Ipv4Address(i.ReadNtohU32());
			m_hopCount = i.ReadNtohU32();
			m_sequenceNumber = i.ReadNtohU32 ();
			m_messageType  = (MessageType)(i.ReadNtohU32());
			
			return GetSerializedSize ();
		}

	}

}
