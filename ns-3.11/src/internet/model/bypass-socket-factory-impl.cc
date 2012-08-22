#include "bypass-socket-factory-impl.h"
#include "bypass.h"
#include "ns3/socket.h"
#include "ns3/assert.h"
#include "ipv4-end-point-demux.h"

namespace ns3
{
	BypassSocketFactoryImpl::BypassSocketFactoryImpl ()
  	: m_bypass (0)
	{
	}
	BypassSocketFactoryImpl::~BypassSocketFactoryImpl ()
	{
  		NS_ASSERT (m_bypass == 0);
	}

	void BypassSocketFactoryImpl::SetBypass (Ptr<BypassProtocol> udp)
	{
  		m_bypass = udp;
	}

	Ptr<Socket> BypassSocketFactoryImpl::CreateSocket (void)
	{
  		return m_bypass->CreateSocket ();
	}

	void BypassSocketFactoryImpl::DoDispose (void)
	{
  		m_bypass = 0;
  		BypassSocketFactory::DoDispose ();
	}

} // namespace ns3
