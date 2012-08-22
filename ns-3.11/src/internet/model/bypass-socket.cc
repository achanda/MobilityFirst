#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "bypass-socket.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BypassSocket);

TypeId
BypassSocket::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BypassSocket")
    .SetParent<Socket> ()
    .AddAttribute ("RcvBufSize",
                   "UdpSocket maximum receive buffer size (bytes)",
                   UintegerValue (131072),
                   MakeUintegerAccessor (&BypassSocket::GetRcvBufSize,
                                         &BypassSocket::SetRcvBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("IpTtl",
                   "socket-specific TTL for unicast IP packets (if non-zero)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BypassSocket::GetIpTtl,
                                         &BypassSocket::SetIpTtl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("IpMulticastTtl",
                   "socket-specific TTL for multicast IP packets (if non-zero)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BypassSocket::GetIpMulticastTtl,
                                         &BypassSocket::SetIpMulticastTtl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("IpMulticastIf",
                   "interface index for outgoing multicast on this socket; -1 indicates to use default interface",
                   IntegerValue (-1),
                   MakeIntegerAccessor (&BypassSocket::GetIpMulticastIf,
                                        &BypassSocket::SetIpMulticastIf),
                   MakeIntegerChecker<int32_t> ())
    .AddAttribute ("IpMulticastLoop",
                   "whether outgoing multicast sent also to loopback interface",
                   BooleanValue (false),
                   MakeBooleanAccessor (&BypassSocket::GetIpMulticastLoop,
                                        &BypassSocket::SetIpMulticastLoop),
                   MakeBooleanChecker ())
    .AddAttribute ("MtuDiscover", "If enabled, every outgoing ip packet will have the DF flag set.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&BypassSocket::SetMtuDiscover,
                                        &BypassSocket::GetMtuDiscover),
                   MakeBooleanChecker ())
  ;
  return tid;
}

BypassSocket::BypassSocket ()
{
}

BypassSocket::~BypassSocket ()
{
}

}; // namespace ns3
