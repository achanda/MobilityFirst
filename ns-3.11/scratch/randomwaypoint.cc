/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mf-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/data-rate.h"

#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ideal-wifi-manager.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/wifi-mode.h"

using namespace ns3;

#define TAG 0.04

std::string phyMode("OfdmRate54Mbps");

NS_LOG_COMPONENT_DEFINE ("SimplePointToPointGstarExample");

NodeContainer nodes;


int main (int argc, char *argv[])
{
	int nNodes = 9;
	int nFlows = 3;

	LogComponentEnable("SimplePointToPointGstarExample",LOG_LEVEL_ALL);
	Config::SetDefault ("ns3::mf::RoutingProtocol::ThruputInterval", TimeValue (Seconds(TAG)));
	
	Config::SetDefault ("ns3::mf::RoutingProtocol::Nodes", UintegerValue(nNodes));
	Config::SetDefault ("ns3::mf::RoutingProtocol::Flows", UintegerValue(nFlows));
	
	SeedManager::SetSeed(12);
	int N = atol(argv[1]); //read in run number from command line
	cout << "Current Seed: " << N << endl;
 	SeedManager::SetRun(N);
	
	NS_LOG_INFO("Create nodes");
	
	nodes.Create (nNodes);
	
	
		// create the channels first without any IP addressing information
	NS_LOG_INFO ("Create channels.");
		
	int nodeSpeed = 5;
	int nodePause = 3;
	
	MobilityHelper mobilityAdhoc;
	
	ObjectFactory pos;
	pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
	pos.Set ("X", RandomVariableValue (UniformVariable (0.0, 300.0)));
	pos.Set ("Y", RandomVariableValue (UniformVariable (0.0, 1500.0)));
	
	Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
	mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
									"Speed", RandomVariableValue (UniformVariable (2.0, nodeSpeed)),
									"Pause", RandomVariableValue (UniformVariable (0.0, nodePause)),
									"PositionAllocator", PointerValue (taPositionAlloc));
	mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
	mobilityAdhoc.Install (nodes);
	
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  	wifiMac.SetType ("ns3::AdhocWifiMac");
  	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	wifiPhy.Set("TxPowerStart",DoubleValue(35)); // 250-300 meter transmission range 
  	wifiPhy.Set("TxPowerEnd",DoubleValue(35));   // 250-300 meter transmission range 
	
	
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());
  	WifiHelper wifi = WifiHelper::Default ();
  	
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",  "ControlMode", StringValue (phyMode), "DataMode", StringValue (phyMode), "RtsCtsThreshold", UintegerValue (0), "NonUnicastMode", StringValue (phyMode));
  	
  	NetDeviceContainer n287 = wifi.Install (wifiPhy, wifiMac, nodes);
	
		// Enable GSTAR
	NS_LOG_INFO ("Enabling GSTAR Routing.");
  	MFHelper gstar;
	
  	Ipv4StaticRoutingHelper staticRouting;
	
  	Ipv4ListRoutingHelper list;
	
  	list.Add (staticRouting,10);
  	list.Add (gstar, 0);
	
  	InternetStackHelper internet;
	
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	
  	internet.Install (nodes);
	
        // Later,add IP addresses.
	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i01 = ipv4.Assign (n287);
	
	Time now = Simulator::Now ();
	
	Simulator::Stop (Seconds (120));
	NS_LOG_INFO ("Run Simulation.");
	
	std::string tr_name = "mf-randomwaypoint";
	
  	Simulator::Run ();
		
  	Simulator::Destroy ();
	NS_LOG_INFO ("Done");  	
	return 0;
}

