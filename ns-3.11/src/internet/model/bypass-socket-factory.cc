#include "bypass-socket-factory.h"
#include "ns3/uinteger.h"

namespace ns3 
{

NS_OBJECT_ENSURE_REGISTERED (BypassSocketFactory);

TypeId BypassSocketFactory::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BypassSocketFactory")
    .SetParent<SocketFactory> ()
  ;
  return tid;
}

} // namespace ns3