#include "transparent-helper.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/transparent.h"

namespace ns3
{
	/*TransparentHelper::TransparentHelper (std::string protocol, Address left, Address right)
	{
		//NS_LOG_FUNCTION(this);
		std::cout << "Helper constructor" << std::endl;
		m_factory.SetTypeId ("ns3::TransparentApplication");
		m_factory.Set ("Protocol", StringValue (protocol));
		m_factory.Set ("Left", AddressValue (left));
		m_factory.Set ("Right", AddressValue (right));
	}*/
	
	TransparentHelper::TransparentHelper (void)
	{
		//NS_LOG_FUNCTION(this);
		std::cout << "Helper constructor" << std::endl;
		m_factory.SetTypeId ("ns3::TransparentApplication");
	}
	
	void
	TransparentHelper::SetAttribute (std::string name, const AttributeValue &value)
	{
		std::cout << "Set atribute" << std::endl;
		//NS_LOG_FUNCTION(this);
		m_factory.Set (name, value);
	}
	
	ApplicationContainer
	TransparentHelper::Install (Ptr<Node> node) const
	{
		//NS_LOG_FUNCTION(this);
		return ApplicationContainer (InstallPriv (node));
	}
	
	ApplicationContainer
	TransparentHelper::Install (std::string nodeName) const
	{
		//NS_LOG_FUNCTION(this);
		Ptr<Node> node = Names::Find<Node> (nodeName);
		return ApplicationContainer (InstallPriv (node));
	}
	
	ApplicationContainer
	TransparentHelper::Install (NodeContainer c) const
	{
		std::cout << "Install" << std::endl;
		//NS_LOG_FUNCTION(this);
		ApplicationContainer apps;
		for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
		{
			apps.Add (InstallPriv (*i));
		}
		
		return apps;
	}
	
	Ptr<TransparentApplication>
	TransparentHelper::InstallPriv (Ptr<Node> node) const
	{
		std::cout << "Install priv" <<std::endl;
		//Ptr<Application> app = m_factory.Create<Application> ();
		Ptr<TransparentApplication> app = m_factory.Create<TransparentApplication> ();
		node->AddApplication (app);
		
		return app;
	}	
}