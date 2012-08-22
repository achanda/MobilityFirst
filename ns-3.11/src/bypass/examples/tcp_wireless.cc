#include <iostream>
#include <fstream>
#include <string>
#include <cassert>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/config-store-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleWifiAdhocExample");

int main (int argc, char *argv[])
{
	LogComponentEnable("SimpleWifiAdhocExample",LOG_LEVEL_ALL);
	uint32_t maxBytes = 100000;

	CommandLine cmd;
	cmd.Parse (argc, argv);
	
	NS_LOG_INFO("Create nodes");
	NodeContainer c;
  	c.Create (7);	  
	
	MobilityHelper mobility;
  	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  	positionAlloc->Add (Vector (10.0, 0.0, 0.0));
	positionAlloc->Add (Vector (20.0, 0.0, 0.0));
	positionAlloc->Add (Vector (30.0, 0.0, 0.0));
	positionAlloc->Add (Vector (40.0, 0.0, 0.0));
	positionAlloc->Add (Vector (50.0, 0.0, 0.0));
	positionAlloc->Add (Vector (60.0, 0.0, 0.0));

		//positionAlloc->Add (Vector (100.0, 0.0, 0.0));
 	mobility.SetPositionAllocator (positionAlloc);
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (c);
	
	NetDeviceContainer devices;
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  	wifiMac.SetType ("ns3::AdhocWifiMac");
  	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  	wifiPhy.SetChannel (wifiChannel.Create ());
  	WifiHelper wifi = WifiHelper::Default ();
  	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"),"RtsCtsThreshold", UintegerValue (0));
  	devices = wifi.Install (wifiPhy, wifiMac, c); 

	InternetStackHelper internet;
	internet.Install (c);

	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  	Ipv4InterfaceContainer i01 = ipv4.Assign (devices);

	// Create router nodes, initialize routing database and set up the routing
  	// tables in the nodes.
  	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	 NS_LOG_INFO ("Create Applications.");

	//	
	// Create a BulkSendApplication and install it on node 0
	//
  	uint16_t port = 9;  // well-known echo port number


  	BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (i01.GetAddress (6), port));
  	// Set the amount of data to send in bytes.  Zero is unlimited.
  	source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  	ApplicationContainer sourceApps = source.Install (c.Get (0));
  	sourceApps.Start (Seconds (0.0));
  	sourceApps.Stop (Seconds (100.0));

	//
	// Create a PacketSinkApplication and install it on node 1
	//
  	PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  	ApplicationContainer sinkApps = sink.Install (c.Get (6));
  	sinkApps.Start (Seconds (0.0));
  	sinkApps.Stop (Seconds (100.0));

	Ptr<FlowMonitor> flowmon;
	FlowMonitorHelper flowmonHelper;
      	flowmon = flowmonHelper.InstallAll ();

	Simulator::Stop (Seconds (100));
	NS_LOG_INFO ("Run Simulation.");
	
  	Simulator::Run ();

	flowmon->SerializeToXmlFile ("simple-wireless-routing.flowmon", false, false);

  	Simulator::Destroy ();

	NS_LOG_INFO ("Done");  	
	return 0;
}



	

