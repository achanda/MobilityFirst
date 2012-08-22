/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats

#include "ns3/assert.h"

#include "mf_header.h"
#include "ns3/log.h"

#define GSTAR_PKT_HEADER_SIZE 12
#define IPV4_ADDRESS_SIZE 4

namespace ns3 
{

namespace mf
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

   if (m_messageType == 1)
        size += lp.GetSerializedSize ();
     
   //if its a LSA packet, add serialized size of LSA packet     
   if (m_messageType == 3)
        size += lsa.GetSerializedSize ();

   //if its a summary vector packet, add serialized size of summary vector packet     
   if (m_messageType == 4 || m_messageType == 5)
        size += summ_vec.GetSerializedSize ();

   //if its a DLSA packet, add serialized size of DLSA packet
   if (m_messageType == 6)
        size += dlsa.GetSerializedSize ();

   //if its a CSYNC packet, add serialized size of CSYNC packet
   if (m_messageType == 7)
        size += csync.GetSerializedSize ();

   //if its a CSYNC ACK packet, add serialized size of CSYNC ACK packet
   if (m_messageType == 8)
        size += csync_ack.GetSerializedSize ();

   //if its a DATA packet, add serialized size of DATA packet
   if (m_messageType == 9)
        size += data.GetSerializedSize ();

   if (m_messageType == 10)
        size += lsa_ack.GetSerializedSize ();

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

  if (m_messageType == 1)
        lp.Serialize (i);
  //if its a LSA packet, serialize the LSA data
  if (m_messageType == 3)
        lsa.Serialize (i);

  //if its a summary vector packet, serialize its data
  if (m_messageType == 4 || m_messageType == 5)
        summ_vec.Serialize (i);

  //if its a DLSA packet, serialize the DLSA data
  if (m_messageType == 6)
        dlsa.Serialize (i);

  //if its a CSYNC packet, serialize the CSYNC data
  if (m_messageType == 7)
        csync.Serialize (i);

  //if its a CSYNC ACK packet, serialize the CSYNC ACK data
  if (m_messageType == 8)
        csync_ack.Serialize (i);

  //if its a DATA packet, serialize the DATA data
  if (m_messageType == 9)
        data.Serialize (i);

  if (m_messageType == 10)
        lsa_ack.Serialize (i);
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

  if (m_messageType == 1)
         size += lp.Deserialize (i);

  //if its a LSA packet, deserialize the LSA data and add its size
  if (m_messageType == 3)
         size += lsa.Deserialize (i);

  //if its a summary vector packet, deserialize its data and add its size
  if (m_messageType == 4 || m_messageType == 5)
         size += summ_vec.Deserialize (i);  

  //if its a DLSA packet, deserialize the DLSA data and add its size
  if (m_messageType == 6)
         size += dlsa.Deserialize (i);

  //if its a CSYNC packet, deserialize the CSYNC data and add its size
  if (m_messageType == 7)
         size += csync.Deserialize (i);
  
  //if its a CSYNC ACK packet, deserialize the CSYNC ACK data and add its size
  if (m_messageType == 8)
         size += csync_ack.Deserialize (i);

  //if its a DATA packet, deserialize the DATA data and add its size
  if (m_messageType == 9)
         size += data.Deserialize (i);

  if (m_messageType == 10)
         size += lsa_ack.Deserialize (i);

  return size; //returns the size of the GSTAR packet
}


uint32_t MessageHeader::LP::GetSerializedSize (void) const
{
        return 6;
}

void MessageHeader::LP::Print (std::ostream &os) const
{
  // TODO
}

void MessageHeader::LP::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
   
  for(int j=0; j<6; j++)
        i.WriteU8 (this->my_mac_addr[j]);
}

uint32_t MessageHeader::LP::Deserialize (Buffer::Iterator start)
{
  uint32_t size;      
  Buffer::Iterator i = start;

  for(int j=0; j<6; j++)
  {
        my_mac_addr[j]  = i.ReadU8 ();
  }
 
  size = 6;

  return size;
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


//returns the serialized size of summary vector packet
uint32_t MessageHeader::summary_vector::GetSerializedSize (void) const
{
        //addition of the addresses all the nodes and the length field
        return this->nodeAddresses.size () * IPV4_ADDRESS_SIZE + 4 + this->nodeseqnums.size() * 8 ;
}

void MessageHeader::summary_vector::Print (std::ostream &os) const
{
  // TODO
}

//serializes summary_vector packet
void MessageHeader::summary_vector::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU32 (this->length); //length of summary vector packet = number of DLSAs of nodes    

  for (std::vector<Ipv4Address>::const_iterator iter = this->nodeAddresses.begin (); iter != this->nodeAddresses.end (); iter++)
  {
      i.WriteHtonU32 (iter->Get ()); //node addresses
  }

   for (std::vector<int64_t>::const_iterator iter = this->nodeseqnums.begin (); iter != this->nodeseqnums.end (); iter++)
  {
      i.WriteHtonU64 (*iter); //node DLSA's squence number
  }
}

//deserializes summary_vector packet
uint32_t MessageHeader::summary_vector::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
 
  this->length = i.ReadU32 (); //length of summary vector packet = number of DLSAs of nodes    

  this->nodeAddresses.clear ();
  this->nodeseqnums.clear ();
  
  this->nodeAddresses.erase (this->nodeAddresses.begin (),this->nodeAddresses.end ());
  for (int n = 0; n < this->length; ++n)
    this->nodeAddresses.push_back (Ipv4Address (i.ReadNtohU32 ())); //node addresses

  for (int n = 0; n < this->length; ++n)
    this->nodeseqnums.push_back (int64_t (i.ReadNtohU64 ())); //node DLSA's squence number

  //addition of the addresses of all the nodes and the length field
  uint32_t messageSize = this->nodeAddresses.size () * IPV4_ADDRESS_SIZE + this->nodeseqnums.size() * 8 + 4;

  return messageSize; //returns the size of summary vector packet
}

//returns the serialized size of DLSA packet
uint32_t MessageHeader::DLSA::GetSerializedSize (void) const
{
        //addition of the addresses and probabilities of all the nodes and the length field
        return this->nodeAddresses.size () * IPV4_ADDRESS_SIZE + this->probabilities.size() * 8 + 4;
}

void MessageHeader::DLSA::Print (std::ostream &os) const
{
  // TODO
}

//serializes DLSA packet
void MessageHeader::DLSA::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU32 (this->length); //length of DLSA packet = number of nodes in DLSA      

  for (std::vector<Ipv4Address>::const_iterator iter = this->nodeAddresses.begin (); iter != this->nodeAddresses.end (); iter++)
  {
      i.WriteHtonU32 (iter->Get ()); //node addresses
  }

  for (std::vector<int64_t>::const_iterator iter = this->probabilities.begin (); iter != this->probabilities.end (); iter++)
  {
      i.WriteHtonU64 (*iter); //node probabilities
  }
}

//deserializes DLSA packet
uint32_t MessageHeader::DLSA::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
 
  this->length = i.ReadU32 (); //length of DLSA packet = number of nodes in DLSA

  this->nodeAddresses.clear ();
  this->probabilities.clear ();
  
  this->nodeAddresses.erase (this->nodeAddresses.begin (),this->nodeAddresses.end ());
  for (int n = 0; n < this->length; ++n)
    this->nodeAddresses.push_back (Ipv4Address (i.ReadNtohU32 ())); //node addresses

  for (int n = 0; n < this->length; ++n)
    this->probabilities.push_back (int64_t (i.ReadNtohU64 ())); //node probabilities

  //addition of the addresses and probabilities of all the nodes and the length field
  uint32_t messageSize = this->nodeAddresses.size () * IPV4_ADDRESS_SIZE + this-> probabilities.size() * 8 + 4;

  return messageSize; //returns the size of DLSA packet
}

//returns the serialized size of CSYNC packet
uint32_t MessageHeader::CSYNC::GetSerializedSize (void) const
{
        //addition of the source address, destination address and chunck packet count field
        return (4 + IPV4_ADDRESS_SIZE * 2);
}

void MessageHeader::CSYNC::Print (std::ostream &os) const
{
  // TODO
}

//serializes CSYNC packet
void MessageHeader::CSYNC::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start; 

  i.WriteHtonU32 (this->src.Get()); //source address
  i.WriteHtonU32 (this->dest.Get()); //destination address
  i.WriteU32 (this->chunck_pkt_count); //length of CSYNC packet = number of packets in that chunck     
}

//deserializes CSYNC packet
uint32_t MessageHeader::CSYNC::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
 
  this->src = Ipv4Address (i.ReadNtohU32 ());
  this->dest = Ipv4Address (i.ReadNtohU32 ());
  this->chunck_pkt_count = i.ReadU32 (); //length of CSYNC packet = number of packets in that chunck

  //addition of the source address, destination address and chunck packet count field
  uint32_t messageSize = 4 + IPV4_ADDRESS_SIZE * 2;

  return messageSize; //returns the size of CSYNC packet
}

//returns the serialized size of CSYNC ACK packet
uint32_t MessageHeader::CSYNC_ACK::GetSerializedSize (void) const
{
        //addition of the source address, destination address, chunck packet count field and bitmap
        return 4 * PAYLOAD_SIZE + 4 + IPV4_ADDRESS_SIZE * 2;
}

void MessageHeader::CSYNC_ACK::Print (std::ostream &os) const
{
  // TODO
}

//serializes CSYNC ACK packet
void MessageHeader::CSYNC_ACK::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU32 (this->src.Get()); //source address
  i.WriteHtonU32 (this->dest.Get());  //destination address
  i.WriteU32 (this->chunck_pkt_count); //number of packets in that chunck  

  //bitmap of the received data packet
  for(int j=0; j<PAYLOAD_SIZE; j++)
        i.WriteU32 (this->bitmap[j]);
           
}

//deserializes CSYNC ACK packet
uint32_t MessageHeader::CSYNC_ACK::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
 
  this->src = Ipv4Address (i.ReadNtohU32 ()); //source address
  this->dest = Ipv4Address (i.ReadNtohU32 ()); //destination address
  this->chunck_pkt_count = i.ReadU32 (); //number of packets in that chunck

  //bitmap of the received data packet
  for(int j=0; j<PAYLOAD_SIZE; j++)
        this->bitmap[j] = i.ReadU32 ();

  //addition of the source address, destination address, chunck packet count and bitmap fields
  uint32_t messageSize = 4 * PAYLOAD_SIZE + 4 + IPV4_ADDRESS_SIZE * 2;

  return messageSize; //returns the size of CSYNC ACK packet
}

//returns the serialized size of DATA packet
uint32_t MessageHeader::DATA::GetSerializedSize (void) const
{
        //addition of the source address, destination address, packet number, payload size and payload fields
        return 2 * IPV4_ADDRESS_SIZE + 2 * 4 + PAYLOAD_SIZE;
}

void MessageHeader::DATA::Print (std::ostream &os) const
{
  // TODO
}

//serializes DATA packet
void MessageHeader::DATA::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU32 (this->src.Get()); //source address
  i.WriteHtonU32 (this->dest.Get()); //destination address
  i.WriteU32 (this->pkt_no); //packet number
  i.WriteU32 (this->payload_size); //payload size
  //payload      
  for(int j=0; j<PAYLOAD_SIZE; j++)
        i.WriteU8 (this->payload[j]);
}

//deserializes DATA packet
uint32_t MessageHeader::DATA::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
 
  this->src = Ipv4Address (i.ReadNtohU32 ()); //source address
  this->dest = Ipv4Address (i.ReadNtohU32 ()); //destination address 
  this->pkt_no = i.ReadU32 (); //packet number
  this->payload_size = i.ReadU32 (); //payload size 

  //payload
  for(int j=0; j<PAYLOAD_SIZE; j++)
        this->payload[j] = i.ReadU8 ();

  //addition of the source address, destination address, packet number, payload size and payload fields
  uint32_t messageSize =  2 * IPV4_ADDRESS_SIZE + 2 * 4 + PAYLOAD_SIZE;

  return messageSize; //returns the size of DATA packet
}

uint32_t MessageHeader::LSA_ACK::GetSerializedSize (void) const
{
        return 4;
}

void MessageHeader::LSA_ACK::Print (std::ostream &os) const
{
  // TODO
}

void MessageHeader::LSA_ACK::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
   
  i.WriteHtonU32 (this->src.Get());
}

uint32_t MessageHeader::LSA_ACK::Deserialize (Buffer::Iterator start)
{
  uint32_t size;      
  Buffer::Iterator i = start;

  this->src = Ipv4Address (i.ReadNtohU32 ()); 
 
  size = 4;

  return size;
}

}

}
