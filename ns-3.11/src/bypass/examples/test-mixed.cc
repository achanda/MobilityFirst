#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/config-store-module.h"
#include "ns3/bypass-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleWifiAdhocExample");

int main (int argc, char *argv[])
{
	LogComponentEnable("SimpleWifiAdhocExample",LOG_LEVEL_ALL);
	
	NS_LOG_INFO("Create nodes");
	
	NodeContainer wiredNodes;
  	wiredNodes.Create (3);
	
	NodeContainer rightWireless;
	rightWireless.Add (wiredNodes.Get(2));
	rightWireless.Create (3);
	
	NodeContainer leftWireless;
	leftWireless.Add (wiredNodes.Get(0));
	leftWireless.Create (1);
	
	NS_LOG_INFO ("Enabling Bypass on all nodes");
  	BypassHelper bypass;
	
  	Ipv4StaticRoutingHelper staticRouting;
	
  	Ipv4ListRoutingHelper list;
	
  	list.Add (staticRouting,10);
  	list.Add (bypass, 0);
	
  	InternetStackHelper internet;
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	internet.Install (wiredNodes);
	internet.Install (leftWireless);
	internet.Install (rightWireless);
	
	Simulator::Stop (Seconds (30));
	NS_LOG_INFO ("Run Simulation.");
	
  	Simulator::Run ();
  	Simulator::Destroy ();
	NS_LOG_INFO ("Done");  	
	return 0;
}