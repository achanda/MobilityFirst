#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "transparent.h"

NS_LOG_COMPONENT_DEFINE ("TransparentApplication");

namespace ns3 
{
	NS_OBJECT_ENSURE_REGISTERED (TransparentApplication);
	
	TypeId TransparentApplication::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::TransparentApplication")
		.SetParent<Application> ()
		.AddConstructor<TransparentApplication> ()
		.AddAttribute ("Protocol", "The type of protocol to use.",
					   TypeIdValue (UdpSocketFactory::GetTypeId ()),
					   MakeTypeIdAccessor (&TransparentApplication::m_tid),
					   MakeTypeIdChecker ())
		.AddAttribute ("Right", "The address of the destination",
					   AddressValue (),
					   MakeAddressAccessor (&TransparentApplication::m_rightPeer),
					   MakeAddressChecker ())
		.AddAttribute ("Left", "The address of the destination",
					   AddressValue (),
					   MakeAddressAccessor (&TransparentApplication::m_leftPeer),
					   MakeAddressChecker ());
		
		return tid;
	}
	
	/*TypeId TransparentApplication::GetInstanceTypeId(void) const
	{
		return GetTypeId();
	}*/
	
	TransparentApplication::TransparentApplication()
	{
		std::cout << "The big bad constructor" << std::endl;
		//m_leftSocket = 0;
		//m_rightSocket = 0;
		m_leftConnected = false;
		m_rightConnected = false;
		m_totSentBytes = 0;
		m_totRecvdBytes = 0;
		
		NS_LOG_FUNCTION(this);
	}
	
	TransparentApplication::~TransparentApplication()
	{
		std::cout << "The big bad destructor" << std::endl;
		NS_LOG_FUNCTION(this);
	}
	
	void TransparentApplication::SetMaxBytes(uint32_t max)
	{
		NS_LOG_FUNCTION(this << max);
		m_maxBytes = max;
	}
	
	Ptr<Socket> TransparentApplication::GetLeftSocket(void)
	{
		NS_LOG_FUNCTION(this);
		return m_leftSocket;
	}
	
	Ptr<Socket> TransparentApplication::GetRightSocket(void)
	{
		NS_LOG_FUNCTION(this);
		return m_rightSocket;
	}
	
	void TransparentApplication::DoDispose(void)
	{
		NS_LOG_FUNCTION(this);
		m_leftSocket = 0;
		m_rightSocket = 0;
		
		Application::DoDispose();
	}
	
	// Methods from Application
	/*void TransparentApplication::StartApplication(void)
	{
		std::cout << "Start app" << std::endl;
		NS_LOG_FUNCTION(this);
		
		// setup the send socket if it does not exist already
		if (!m_rightSocket)
			m_rightSocket = Socket::CreateSocket(GetNode() /*GetObject<Node> (), UdpSocketFactory::GetTypeId());
		m_rightSocket->Bind();
		m_rightSocket->Connect(m_rightPeer);
		//m_rightSocket->ShutDownRecv();
		//m_rightSocket->SetConnectCallback (
		//								   MakeCallback (&TransparentApplication::RightConnectionSucceeded, this),
		//								   MakeCallback (&TransparentApplication::RightConnectionFailed, this));
		//m_rightSocket->SetSendCallback (
		//								MakeCallback (&TransparentApplication::DataSend, this));
		
		// setup the receive socket if it does not exist
		if (!m_leftSocket)
			m_leftSocket = Socket::CreateSocket(GetNode() /*GetObject<Node> (), UdpSocketFactory::GetTypeId());
		m_leftSocket->Bind(m_local);
		m_leftSocket->Listen();
		//m_leftSocket->ShutDownSend();
		m_leftSocket->SetRecvCallback(MakeCallback(&TransparentApplication::DataRead, this));
		//m_leftSocket->SetAcceptCallback(
		//MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
		//								MakeCallback(&TransparentApplication::HandleAccept, this));
		//m_leftSocket->SetCloseCallbacks (
		//								 MakeCallback(&TransparentApplication::HandlePeerClose, this),
		//								 MakeCallback(&TransparentApplication::HandlePeerError, this));
		//if (m_rightConnected)
		//	SendData();
	}*/
	void TransparentApplication::StartApplication (void)
	{
        NS_LOG_DEBUG ("Starting app";
		
		Ipv4Address loopback ("127.0.0.1");
		//bool canRungstar = false;
		
		for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
        {
			Ipv4Address addr = m_ipv4->GetAddress (i, 0).GetLocal ();
			
			if (addr == loopback)
                continue;
			
			// Create a socket to listen only on this interface
			Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),UdpSocketFactory::GetTypeId ());
			InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), 9);
			
			if(addr == "10.1.1.2")
				socket->SetRecvCallback (MakeCallback (&TransparentApplication::DataRead,  this));
			if (socket->Bind (inetAddr))
            {
				NS_FATAL_ERROR ("Failed to bind() GSTAR socket");
            }
			socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
			
			m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
        }
		
		NS_LOG_DEBUG ("GSTAR on node " << m_mainAddress << " started");
	}
	
	void TransparentApplication::StopApplication(void)
	{
		std::cout << "Stop app" << std::endl;
		NS_LOG_FUNCTION(this);
		
		if (m_leftSocket != 0)
		{
			m_leftSocket->Close();
			//m_leftSocket->SetRecvCallbackn(MakeNullCallback<void, Ptr<Socket>> ());
			m_leftConnected = false;
		}
		
		if (m_rightSocket != 0)
		{
			m_rightSocket->Close();
			m_rightConnected = false;
		}
	}

	void TransparentApplication::DataRead (Ptr<Socket> socket)
	{
		std::cout << "Data read" << std::endl;
		NS_LOG_FUNCTION (this << socket);
		Ptr<Packet> packet;
		Address from;
		while (packet = socket->RecvFrom (from))
		{
			std::cout << "Packet received " << std::endl;
			if (packet->GetSize () == 0)
			{ //EOF
				break;
			}
			if (InetSocketAddress::IsMatchingType (from))
			{
				m_totRecvdBytes += packet->GetSize ();
				InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
				//NS_LOG_INFO ("Received " << packet->GetSize () << " bytes from " <<
				//			 address.GetIpv4 () << " [" << address << "]"
				//			 << " total Rx " << m_totalRx);
				//cast address to void , to suppress 'address' set but not used 
				//compiler warning in optimized builds
				//(void) address;
			}
			m_rightSocket->SendTo(packet,0,m_rightPeer);
			std::cout << "Packet sent " << std::endl;
			//m_rxTrace (packet, from);
		}
	}
}
