#ifndef TRANSPARENT_HELPER_
#define TRANSPARENT_HELPER_

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/transparent.h"

namespace ns3
{
	class TransparentHelper
	{
		public:
		//TransparentHelper(std::string protocol, Address left, Address right);
		TransparentHelper(void);
		void SetAttribute (std::string name, const AttributeValue &value);
		ApplicationContainer Install (NodeContainer c) const;
		ApplicationContainer Install (Ptr<Node> node) const;
		ApplicationContainer Install (std::string nodeName) const;
		
		private:
		//Ptr<Application> InstallPriv (Ptr<Node> node) const;
		Ptr<TransparentApplication> InstallPriv (Ptr<Node> node) const;
		std::string m_protocol;
		Address m_left;
		Address m_right;
		ObjectFactory m_factory;
	};
}

#endif