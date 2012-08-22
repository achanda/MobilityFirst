/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
//same as AODV helper

#ifndef GSTARHELPER_H_
#define GSTARHELPER_H_

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

#include <map>
#include <set>

namespace ns3
{
/**
 * \ingroup gstar
 * \brief Helper class that adds GSTAR routing to nodes.
 */
class GSTARHelper : public Ipv4RoutingHelper
{
public:
  	GSTARHelper();

        /**
   	* \internal
   	* \returns pointer to clone of this OlsrHelper 
   	* 
   	* This method is mainly for internal use by the other helpers;
   	* clients are expected to free the dynamic memory allocated by this method
   	*/
  	GSTARHelper* Copy (void) const;

  	/**
   	* \param node the node on which the routing protocol will run
   	* \returns a newly-created routing protocol
   	*
   	* This method will be called by ns3::InternetStackHelper::Install
   	* 
  	*/
  	virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;
  
   	/**
   	* \param name the name of the attribute to set
   	* \param value the value of the attribute to set.
   	*
   	* This method controls the attributes of ns3::gstar::RoutingProtocol
   	*/
  	void Set (std::string name, const AttributeValue &value);

private:
  	ObjectFactory m_agentFactory;
};

}
#endif /* GSTARHELPER_H_ */
