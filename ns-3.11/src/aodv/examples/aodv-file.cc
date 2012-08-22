#include "ns3/core-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
	NodeContainer nodes;

	// create an array of empty nodes for testing purposes 
	//nodes.Create (10);

	Ns2MobilityHelper ns2 = Ns2MobilityHelper ("scen-670x670-50-600-20-0");
	ns2.Install ();

	MobilityHelper mobility;
	mobility.EnableAsciiAll (std::cout);

	Simulator::Run ();

	return 0;
} 

