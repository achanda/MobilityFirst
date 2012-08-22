#ifndef BYPASS_SOCKET_FACTORY_IMPL_H
#define BYPASS_SOCKET_FACTORY_IMPL_H

#include "ns3/bypass-socket-factory.h"
#include "ns3/ptr.h"

namespace ns3
{
	class BypassProtocol;

	class BypassSocketFactoryImpl : public BypassSocketFactory
	{
		public:
  		BypassSocketFactoryImpl ();
  		virtual ~BypassSocketFactoryImpl ();

  		void SetBypass (Ptr<BypassProtocol> bypass);

  /**
   * \brief Implements a method to create a Udp-based socket and return
   * a base class smart pointer to the socket.
   * \internal
   *
   * \return smart pointer to Socket
   */
  	virtual Ptr<Socket> CreateSocket (void);

	protected:
  	virtual void DoDispose (void);

	private:
  	Ptr<BypassProtocol> m_bypass;
};

} // namespace ns3

#endif /* BYPASS_SOCKET_FACTORY_IMPL_H */
