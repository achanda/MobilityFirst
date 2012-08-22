#ifndef __transparent_h__
#define __transparent_h__

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4.h"

namespace ns3
{
	class Address;
	class RandomVariable;
	class Socket;
	
	class TransparentApplication : public Application
	{
		public:
			static TypeId GetTypeId (void);
			//virtual TypeId GetInstanceTypeId (void) const;
			TransparentApplication ();
			virtual ~TransparentApplication ();
		
			//Sets the maximum number of bytes to be transferred
			void SetMaxBytes (uint32_t maxBytes);
		
			//Returns a pointer to the current receive socket
			Ptr<Socket> GetLeftSocket (void);
			Ptr<Socket> GetRightSocket (void);
		
		protected:
			virtual void DoDispose (void);
		
		private:
			// inherited from Application base class.
			virtual void StartApplication (void);    // Called at time specified by Start
			virtual void StopApplication (void);     // Called at time specified by Stop
		
			//Sends data!
			void SendData ();
		
			// (left_peer) ------> [leftSocket](transparent)[rightSocket] -----> (right_peer)
			Ptr<Socket>     m_leftSocket;       // Associated receive socket (S)
			Ptr<Socket>		m_rightSocket;		   // Associated send socket (D)
			Address         m_leftPeer;         // Address of peer which sends data (S)
			Address			m_rightPeer;			   // Address of peer to which data is being sent (D)
			bool            m_leftConnected;    // True if connected to S
			bool			m_rightConnected;      // True if connected to D
			uint32_t        m_sendSize;     // Size of data to send each time
			uint32_t        m_maxBytes;     // Limit total number of bytes sent
			uint32_t        m_totSentBytes;     // Total bytes sent so far
			uint32_t		m_totRecvdBytes;	// Total bytes received so far
			TypeId          m_tid;
			Address			m_local;			//Local address to bind to (from PacketSink)
			Ptr<Ipv4>		m_ipv4;
			//Time			abhishek;
			//TracedCallback<Ptr<const Packet> > m_txTrace;
		
		private:
			//void LeftConnectionSucceeded (Ptr<Socket> socket);
			void RightConnectionSucceeded (Ptr<Socket> socket);
			//void LeftConnectionFailed (Ptr<Socket> socket);
			void RightConnectionFailed (Ptr<Socket> socket);
			void HandleAccept (Ptr<Socket>, const Address& from);
			void HandlePeerClose (Ptr<Socket>);
			void HandlePeerError (Ptr<Socket>);
		
			void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback
			void DataRead (Ptr<Socket>);
			void Ignore (Ptr<Socket> socket);
	};
}

#endif