#ifndef BYPASS_SOCKET_FACTORY_H
#define BYPASS_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 
{

	class Socket;

/**
 * \ingroup socket
 *
 * \brief API to create UDP socket instances 
 *
 * This abstract class defines the API for UDP socket factory.
 * All UDP implementations must provide an implementation of CreateSocket
 * below.
 * 
 * \see UdpSocketFactoryImpl
 */
	class BypassSocketFactory : public SocketFactory
	{
	public:
  		static TypeId GetTypeId (void);
	};

} // namespace ns3

#endif /* BYPASS_SOCKET_FACTORY_H */
