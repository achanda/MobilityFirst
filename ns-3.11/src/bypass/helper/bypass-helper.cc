/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//same as AODV helper
#include "bypass-helper.h"
#include "ns3/bypass.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

using namespace std;

namespace ns3
{
	BypassHelper::BypassHelper() : Ipv4RoutingHelper ()
	{
		//cout << "In bypass helper" << endl;
  		m_agentFactory.SetTypeId ("ns3::Bypass");
	}

	BypassHelper* BypassHelper::Copy (void) const 
	{
  		return new BypassHelper (*this); 
	}

	Ptr<Ipv4RoutingProtocol> BypassHelper::Create (Ptr<Node> node) const
	{
		//cout << "In helper create" << endl;
  		Ptr<Bypass> agent = m_agentFactory.Create<Bypass> ();
  		node->AggregateObject (agent);
  		return agent;
	}

	void BypassHelper::Set (std::string name, const AttributeValue &value)
	{
  		m_agentFactory.Set (name, value);
	}

}
