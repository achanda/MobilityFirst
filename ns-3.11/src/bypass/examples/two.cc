	//#include "ns3/core-module.h" 
	//#include "ns3/common-module.h" 
	//#include "ns3/node-module.h" 
	//#include "ns3/helper-module.h" 
	//#include "ns3/mobility-module.h" 
	//#include "ns3/contrib-module.h" 

#include <fstream> 
#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/config-store-module.h"
#include "ns3/bypass-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"

using namespace ns3; 
using namespace std; 
Vector 
GetPosition (Ptr<Node> node) 
{ 
	Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> (); 
	return mobility->GetPosition (); 
} 

static void 
AdvancePosition (NodeContainer stas) 
{ 
	Ptr<Node> node = stas.Get (0); 
	cout << Simulator::Now () <<" "<<  GetPosition (node) << std::endl; 
	Simulator::Schedule (Seconds (1), &AdvancePosition, stas); 
} 

int main (int argc, char *argv[]) 
{ 
	std::string filename = string("data.txt");
	Ns2MobilityHelper mobility(filename);
	NodeContainer stas; 
	stas.Create (1); 
	mobility.Install (); 
	Simulator::Schedule (Seconds (0), &AdvancePosition, stas); 
	Simulator::Stop (Seconds (10.0)); 
	Simulator::Run (); 
	Simulator::Destroy (); 
	return 0; 
} 