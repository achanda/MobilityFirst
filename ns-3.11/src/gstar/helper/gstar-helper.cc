/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//same as AODV helper
#include "gstar-helper.h"
#include "ns3/gstar-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

	GSTARHelper::GSTARHelper() : Ipv4RoutingHelper ()
	{
  		m_agentFactory.SetTypeId ("ns3::gstar::RoutingProtocol");
	}

	GSTARHelper* GSTARHelper::Copy (void) const 
	{
  		return new GSTARHelper (*this); 
	}

	Ptr<Ipv4RoutingProtocol> GSTARHelper::Create (Ptr<Node> node) const
	{
  		Ptr<gstar::RoutingProtocol> agent = m_agentFactory.Create<gstar::RoutingProtocol> ();
  		node->AggregateObject (agent);
  		return agent;
	}

	void GSTARHelper::Set (std::string name, const AttributeValue &value)
	{
  		m_agentFactory.Set (name, value);
	}

}
