/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//same as AODV helper
#include "transparent-helper.h"
#include "ns3/transparent.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{
	
	MFHelperA::MFHelperA() //: Ipv4RoutingHelper ()
	{
  		m_agentFactory.SetTypeId ("ns3::mf::RoutingProtocol");
	}
	
	MFHelperA* MFHelperA::Copy (void) const 
	{
  		return new MFHelperA (*this); 
	}
	
	Ptr<Ipv4RoutingProtocol> MFHelperA::Create (Ptr<Node> node) const
	{
  		Ptr<mf::RoutingProtocol> agent = m_agentFactory.Create<mf::RoutingProtocol> ();
  		node->AggregateObject (agent);
  		return agent;
	}
	
	void MFHelperA::Set (std::string name, const AttributeValue &value)
	{
  		m_agentFactory.Set (name, value);
	}
	
}
