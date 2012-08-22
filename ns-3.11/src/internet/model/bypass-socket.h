#ifndef BYPASS_SOCKET_H__
#define BYPASS_SOCKET_H__

#include "ns3/socket.h"
#include "ns3/traced-callback.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

namespace ns3 
{

class Node;
class Packet;

/**
 * \ingroup socket
 *
 * \brief (abstract) base class of all UdpSockets
 *
 * This class exists solely for hosting UdpSocket attributes that can
 * be reused across different implementations, and for declaring
 * UDP-specific multicast API.
 */
class BypassSocket : public Socket
{
public:
  static TypeId GetTypeId (void);
  BypassSocket (void);
  virtual ~BypassSocket (void);
    virtual int MulticastJoinGroup (uint32_t interface, const Address &groupAddress) = 0;
  virtual int MulticastLeaveGroup (uint32_t interface, const Address &groupAddress) = 0;

  private:
  // Indirect the attribute setting and getting through private virtual methods
  virtual void SetRcvBufSize (uint32_t size) = 0;
  virtual uint32_t GetRcvBufSize (void) const = 0;
  virtual void SetIpTtl (uint8_t ipTtl) = 0;
  virtual uint8_t GetIpTtl (void) const = 0;
  virtual void SetIpMulticastTtl (uint8_t ipTtl) = 0;
  virtual uint8_t GetIpMulticastTtl (void) const = 0;
  virtual void SetIpMulticastIf (int32_t ipIf) = 0;
  virtual int32_t GetIpMulticastIf (void) const = 0;
  virtual void SetIpMulticastLoop (bool loop) = 0;
  virtual bool GetIpMulticastLoop (void) const = 0;
  virtual void SetMtuDiscover (bool discover) = 0;
  virtual bool GetMtuDiscover (void) const = 0;
};

} //namespace ns3

#endif /* UDP_SOCKET_H */