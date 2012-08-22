/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats
//GSTAR packet format: TYPE,SOURCE ADDRESS,SEQUENCE NUMBER
//LSA packet has an addition to the above
//...LENGTH(say N),NEIGHBOR-1 ADDR,NEIGHBOR-1 SETT,NEIGHBOR-1 LETT,...,NEIGHBOR-N ADDR,NEIGHBOR-N SETT,NEIGHBOR-N LETT

#include "ns3/assert.h"

#include "gstar_header.h"
#include "ns3/log.h"

#define GSTAR_PKT_HEADER_SIZE 12
#define IPV4_ADDRESS_SIZE 4

namespace ns3 
{

namespace gstar
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
 	static TypeId tid = TypeId ("ns3::gstar::MessageHeader")
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
   uint32_t size = GSTAR_PKT_HEADER_SIZE;

   //if its a LSA packet, add serialized size of LSA packet     
   if (m_messageType == 3)
        size += lsa.GetSerializedSize ();

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
  i.WriteHtonU32 (m_messageType); //message type
  i.WriteHtonU32 (m_SequenceNumber); //message sequence number
  i.WriteHtonU32 (m_srcAddr.Get ()); //message source address  

  //if its a LSA packet, serialize the LSA data
  if (m_messageType == 3)
        lsa.Serialize (i);

}

//deserializes GSTAR packet
uint32_t MessageHeader::Deserialize (Buffer::Iterator start)
{
  uint32_t size;      
  Buffer::Iterator i = start;
  m_messageType  = i.ReadNtohU32 (); //message type
  m_SequenceNumber = i.ReadNtohU32 (); //message sequence number
  m_srcAddr = Ipv4Address (i.ReadNtohU32 ()); //message source address
  size = GSTAR_PKT_HEADER_SIZE; //size = addition of the above sizes

  //if its a LSA packet, deserialize the LSA data and add its size
  if (m_messageType == 3)
         size += lsa.Deserialize (i); 

  return size; //returns the size of the GSTAR packet
}

//returns the serialized size of LSA packet
uint32_t MessageHeader::LSA::GetSerializedSize (void) const
{
        //addition of the addresses, SETTs and LETTs of all the neighbors and the length field
        return this->neighborAddresses.size () * IPV4_ADDRESS_SIZE + this->neighborSETT.size() * 8 + this->neighborLETT.size() * 8 + 4;
}

void MessageHeader::LSA::Print (std::ostream &os) const
{
  // TODO
}

//serializes LSA packet
void MessageHeader::LSA::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU32 (this->length); //length of LSA packet = number of neighbors in LSA      

  for (std::vector<Ipv4Address>::const_iterator iter = this->neighborAddresses.begin (); iter != this->neighborAddresses.end (); iter++)
  {
      i.WriteHtonU32 (iter->Get ()); //neighbor addresses
  }

  for (std::vector<int64_t>::const_iterator iter = this->neighborSETT.begin (); iter != this->neighborSETT.end (); iter++)
  {
      i.WriteHtonU64 (*iter); //neighbor SETTs
  }

  for (std::vector<int64_t>::const_iterator iter = this->neighborLETT.begin (); iter != this->neighborLETT.end (); iter++)
  {
      i.WriteHtonU64 (*iter); //neighbor LETTs
  }
}

//deserializes LSA packet
uint32_t MessageHeader::LSA::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
 
  this->length = i.ReadU32 (); //length of LSA packet = number of neighbors in LSA

  this->neighborAddresses.clear ();
  this->neighborSETT.clear ();
  this->neighborLETT.clear ();

  this->neighborAddresses.erase (this->neighborAddresses.begin (),this->neighborAddresses.end ());
  for (int n = 0; n < this->length; ++n)
    this->neighborAddresses.push_back (Ipv4Address (i.ReadNtohU32 ())); //neighbor addresses

  for (int n = 0; n < this->length; ++n)
    this->neighborSETT.push_back (int64_t (i.ReadNtohU64 ())); //neighbor SETTs

  for (int n = 0; n < this->length; ++n)
    this->neighborLETT.push_back (int64_t (i.ReadNtohU64 ())); //neighbor LETTs

  //addition of the addresses, SETTs and LETTs of all the neighbors and the length field
  uint32_t messageSize = this->neighborAddresses.size () * IPV4_ADDRESS_SIZE + this->neighborSETT.size() * 8 + this->neighborLETT.size() * 8 + 4;

  return messageSize; //returns the size of LSA packet
}


}

}
