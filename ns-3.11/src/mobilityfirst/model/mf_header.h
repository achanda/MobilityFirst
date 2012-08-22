/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats

#ifndef MF_HEADER_H
#define MF_HEADER_H

#include <stdint.h>
#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"

#define PAYLOAD_SIZE 256

namespace ns3 
{
	namespace mf
	{
		class MessageHeader : public Header
		{
			public:

  			MessageHeader ();
  			virtual ~MessageHeader ();

			//sets type of message
  			void SetMessageType (uint32_t messageType)
  			{
    				m_messageType = messageType;
  			}
  			
                        //sets source addr of message
  			void SetMessageAddr (Ipv4Address srcAddr)
  			{
    				m_srcAddr = srcAddr;
  			}

			//sets sequence number of packet
			void SetSequenceNumber (uint32_t SequenceNumber)
  			{
    				m_SequenceNumber = SequenceNumber;
  			}

			//gives type of message
			uint32_t GetMessageType () const
  			{
    				return m_messageType;
  			}

			//gives sequence number of packet
			uint32_t GetSequenceNumber () const
  			{
			    	return m_SequenceNumber;
  			}

                        //gives source addr of message
  			Ipv4Address GetMessageAddr () const
  			{
    				return m_srcAddr;
  			}

			//sets Parent, constructor and attributes (if required)
			//used in both AODV and OLSR codes
			static TypeId GetTypeId (void);
                        virtual TypeId GetInstanceTypeId (void) const;
                        virtual void Print (std::ostream &os) const;
                        virtual uint32_t GetSerializedSize (void) const; //gives the size ok GSTAR packet header
                        virtual void Serialize (Buffer::Iterator start) const; //serializes GSTAR packet header
                        virtual uint32_t Deserialize (Buffer::Iterator start); //deserializes GSTAR packet header
			

                        struct LP
                        {   
                                uint8_t my_mac_addr[6];
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the LP packet
                                void Serialize (Buffer::Iterator start) const; //serializes the LP packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the LP packet
                        };

                        LP& GetLP ()
                        {    
                            return lp;
                        }

                        //packet format for LSAs
                        struct LSA
                        {   
                                int length; //number of neighbors in the LSA packet
                                std::vector<Ipv4Address> neighborAddresses; //a vector of neighbor addresses
                                std::vector<int64_t> neighborSETT; //a vector of neighbor SETTs
                                std::vector<int64_t> neighborLETT; //a vector of neighbor LETTs
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the LSA packet
                                void Serialize (Buffer::Iterator start) const; //serializes the LSA packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the LSA packet
                        };

                        //returns the LSA packet
                        LSA& GetLSA ()
                        {    
                            return lsa;
                        }

                        //packet format for summary vector 
                        struct summary_vector
                        {   
                                int length; //number of neighbors in the LSA packet
                                std::vector<Ipv4Address> nodeAddresses; //a vector of node addresses
                                std::vector<int64_t> nodeseqnums; //a vector of node DLSA's sequence numbbers
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the summary vector packet
                                void Serialize (Buffer::Iterator start) const; //serializes the summary vector packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the summary vector packet
                        };

                        //returns the summary vector packet
                        summary_vector& Get_summary_vector ()
                        {    
                            return summ_vec;
                        }

                        //packet format for DLSAs
                        struct DLSA
                        {   
                                int length; //number of neighbors in the LSA packet
                                std::vector<Ipv4Address> nodeAddresses; //a vector of neighbor addresses
                                std::vector<int64_t> probabilities; //a vector of probabilities
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the LSA packet
                                void Serialize (Buffer::Iterator start) const; //serializes the LSA packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the LSA packet
                        };

                        //returns the DLSA packet
                        DLSA& GetDLSA ()
                        {    
                            return dlsa;
                        }

                        //packet format for CSYNC
                        struct CSYNC
                        {   
                                Ipv4Address src; //source address
                                Ipv4Address dest; //destination address
                                int chunck_pkt_count; //number of data packets in this chunck
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the CSYNC packet
                                void Serialize (Buffer::Iterator start) const; //serializes the CSYNC packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the CSYNC packet
                        };

                        //returns the CSYNC packet
                        CSYNC& GetCSYNC ()
                        {    
                            return csync;
                        }

                        //packet format for CSYNC ACK
                        struct CSYNC_ACK
                        {   
                                Ipv4Address src; //source address
                                Ipv4Address dest; //destination address
                                int chunck_pkt_count; //number of data packets in this chunck
                                int bitmap[PAYLOAD_SIZE]; //bitmap of data packets received
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the CSYNC ACK packet
                                void Serialize (Buffer::Iterator start) const; //serializes the CSYNC ACK packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the CSYNC ACK packet
                        };

                        //returns the CSYNC_ACK packet
                        CSYNC_ACK& GetCSYNC_ACK ()
                        {    
                            return csync_ack;
                        }

                        //packet format for DATA
                        struct DATA
                        {   
                                Ipv4Address src; //source address
                                Ipv4Address dest; //destination address
                                int pkt_no; // packet number
                                int payload_size; //payload size
                                char payload[PAYLOAD_SIZE]; //payload
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the DATA packet
                                void Serialize (Buffer::Iterator start) const; //serializes the DATA packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the DATA packet
                        };

                        //returns the DATA packet
                        DATA& GetDATA ()
                        {    
                            return data;
                        }

                        struct LSA_ACK
                        {   
                                Ipv4Address src;
                                void Print (std::ostream &os) const;
                                uint32_t GetSerializedSize (void) const; //gives the size of the LP packet
                                void Serialize (Buffer::Iterator start) const; //serializes the LP packet
                                uint32_t Deserialize (Buffer::Iterator start); //deserializes the LP packet
                        };

                        LSA_ACK& GetLSA_ACK ()
                        {    
                            return lsa_ack;
                        }

                        private:
 	 		
			uint32_t m_messageType; //type of messsage
  			uint32_t m_SequenceNumber; //sequence number of message
                        Ipv4Address m_srcAddr; //source address of message

                      
                        LP lp;
                        LSA lsa; //LSA packet structure
                        summary_vector summ_vec; // summary vector packet structure
                        DLSA dlsa; //DLSA packet structure
                        CSYNC csync; //CSYNC packet structure
                        CSYNC_ACK csync_ack; //CSYNC ACK packet structure
                        DATA data; //DATA packet structure
                        LSA_ACK lsa_ack;


		};

                
       	}
}

#endif /* MF_HEADER_H */




