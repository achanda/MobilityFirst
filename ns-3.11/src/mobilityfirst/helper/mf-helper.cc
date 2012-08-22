/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//same as AODV helper
#include "mf-helper.h"
#include "ns3/mobilityfirst.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

	MFHelper::MFHelper() : Ipv4RoutingHelper ()
	{
  		m_agentFactory.SetTypeId ("ns3::mf::RoutingProtocol");
	}

	MFHelper* MFHelper::Copy (void) const 
	{
  		return new MFHelper (*this); 
	}

	Ptr<Ipv4RoutingProtocol> MFHelper::Create (Ptr<Node> node) const
	{
  		Ptr<mf::RoutingProtocol> agent = m_agentFactory.Create<mf::RoutingProtocol> ();
  		node->AggregateObject (agent);
  		return agent;
	}

	void MFHelper::SetAttribute (std::string name, const AttributeValue &value)
	{
  		m_agentFactory.Set (name, value);
	}

}
