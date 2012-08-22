/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

//packet formats
//GSTAR packet format: TYPE,SOURCE ADDRESS,SEQUENCE NUMBER
//LSA packet has an addition to the above
//...LENGTH(say N),NEIGHBOR-1 ADDR,NEIGHBOR-1 SETT,NEIGHBOR-1 LETT,...,NEIGHBOR-N ADDR,NEIGHBOR-N SETT,NEIGHBOR-N LETT

#ifndef GSTAR_HEADER_H
#define GSTAR_HEADER_H

#include <stdint.h>
#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

namespace ns3 
{
	namespace gstar
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

			//sets sequence number of link probe packet
			void SetSequenceNumber (uint32_t SequenceNumber)
  			{
    				m_SequenceNumber = SequenceNumber;
  			}

			//gives type of message
			uint32_t GetMessageType () const
  			{
    				return m_messageType;
  			}

			//gives sequence number of link probe packet
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
			
                        private:
 	 		
			uint32_t m_messageType; //type of messsage
  			uint32_t m_SequenceNumber; //sequence number of message
                        Ipv4Address m_srcAddr; //source address of message

                      
                        LSA lsa; //LSA packet structure
    


		};

                
       	}
}

#endif /* GSTAR_HEADER_H */




