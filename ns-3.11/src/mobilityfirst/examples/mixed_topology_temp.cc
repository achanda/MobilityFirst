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

std::string phyMode("OfdmRate54Mbps");

NS_LOG_COMPONENT_DEFINE ("SimplePointToPointGstarExample");

NodeContainer nodes;
void MoveNodeNear ();

void MoveNodeAway ()
{
	cout << "Moving Node Away" << endl;
	
	Ptr<Node> node_1 = nodes.Get (8);
  	Ptr<MobilityModel> mob_1 = node_1->GetObject<MobilityModel> ();
  	mob_1->SetPosition(Vector (400.0, 0.0, 0.0));
	
		
	Time now = Simulator::Now ();
	
	int jitter_var = 5;
	UniformVariable xlp (-jitter_var,jitter_var);
	cout << xlp.GetValue() << endl;
	
	Simulator::Schedule (Seconds (15.0 + xlp.GetValue()), &MoveNodeNear);
	
}

void MoveNodeNear ()
{
	cout << "Moving Node Near" << endl;
	
	Ptr<Node> node_1 = nodes.Get (8);
  	Ptr<MobilityModel> mob_1 = node_1->GetObject<MobilityModel> ();
  	mob_1->SetPosition(Vector (100.0, 0.0, 0.0));
	
	Time now = Simulator::Now ();
	
	int jitter_var = 5;
	UniformVariable xlp (-jitter_var,jitter_var);
	cout << xlp.GetValue() << endl;
	
	Simulator::Schedule (Seconds (15.0 + xlp.GetValue()), &MoveNodeAway);
}


int main (int argc, char *argv[])
{
	LogComponentEnable("SimplePointToPointGstarExample",LOG_LEVEL_ALL);
	Config::SetDefault ("ns3::mf::RoutingProtocol::ThruputInterval", TimeValue (Seconds(0.005)));
	
	SeedManager::SetSeed(12);
	int N = atol(argv[1]); //read in run number from command line
	cout << "Current Seed: " << N << endl;
 	SeedManager::SetRun(N);
	
	NS_LOG_INFO("Create nodes");
	
	nodes.Create (9);
	
		//left square
	/*NodeContainer n01 = NodeContainer (nodes.Get (0), nodes.Get (1));
	NodeContainer n12 = NodeContainer (nodes.Get (1), nodes.Get (2));
	NodeContainer n23 = NodeContainer (nodes.Get (2), nodes.Get (3));
	NodeContainer n30 = NodeContainer (nodes.Get (3), nodes.Get (0));
	
		//right square
	NodeContainer n45 = NodeContainer (nodes.Get (4), nodes.Get (5));
	NodeContainer n56 = NodeContainer (nodes.Get (5), nodes.Get (6));
	NodeContainer n67 = NodeContainer (nodes.Get (6), nodes.Get (7));
	NodeContainer n74 = NodeContainer (nodes.Get (7), nodes.Get (4));
	
	//wireless bridge
	NodeContainer n287 = NodeContainer (nodes.Get (2), nodes.Get (8),nodes.Get (7));*/
	
	
		// create the channels first without any IP addressing information
	NS_LOG_INFO ("Create channels.");
  	
	MobilityHelper mobility;
  	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  	positionAlloc->Add (Vector (0.0, 200.0, 0.0));
  	positionAlloc->Add (Vector (100.0, 200.0, 0.0));
	positionAlloc->Add (Vector (100.0, 100.0, 0.0));
	positionAlloc->Add (Vector (0.0, 100.0, 0.0));
  	positionAlloc->Add (Vector (400.0, 200.0, 0.0));
  	positionAlloc->Add (Vector (500.0, 200.0, 0.0));
  	positionAlloc->Add (Vector (100.0, 500.0, 0.0));
  	positionAlloc->Add (Vector (100.0, 400.0, 0.0));
  	positionAlloc->Add (Vector (100.0, 0.0, 0.0));

 	mobility.SetPositionAllocator (positionAlloc);
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (nodes);
	
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
	
	
	/*PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", StringValue ("54Mbps"));
  	
	NetDeviceContainer nd01 = p2p.Install (n01);
	NetDeviceContainer nd12 = p2p.Install (n12);
	NetDeviceContainer nd23 = p2p.Install (n23);
	NetDeviceContainer nd30 = p2p.Install (n30);
	
	NetDeviceContainer nd45 = p2p.Install (n45);
	NetDeviceContainer nd56 = p2p.Install (n56);
	NetDeviceContainer nd67 = p2p.Install (n67);
	NetDeviceContainer nd74 = p2p.Install (n74);*/
	
	/*CsmaHelper csma;
	 csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
	 csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
	 NetDeviceContainer csmaDevices = csma.Install (nodes);*/
		// Enable GSTAR
	NS_LOG_INFO ("Enabling GSTAR Routing.");
  	MFHelper gstar;
    gstar->dtn(true);
	
  	Ipv4StaticRoutingHelper staticRouting;
	
  	Ipv4ListRoutingHelper list;
	
  	list.Add (staticRouting,10);
  	list.Add (gstar, 0);
	
  	InternetStackHelper internet;
	
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	
  	internet.Install (nodes);
        /////internet.Install (wireless);
	
        // Later,add IP addresses.
	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i01 = ipv4.Assign (n287);
	/*ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);
	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer i23 = ipv4.Assign (nd23);
	ipv4.SetBase ("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer i15 = ipv4.Assign (nd15);
	ipv4.SetBase ("10.1.5.0", "255.255.255.0");
	Ipv4InterfaceContainer i34 = ipv4.Assign (nd34);
	ipv4.SetBase ("10.1.6.0", "255.255.255.0");
	Ipv4InterfaceContainer i36 = ipv4.Assign (nd36);
	ipv4.SetBase ("10.1.7.0", "255.255.255.0");
	Ipv4InterfaceContainer i6748 = ipv4.Assign (nd6748);*/
	
	Time now = Simulator::Now ();
	Simulator::Schedule (Seconds (30), &MoveNodeAway);
	
	Simulator::Stop (Seconds (90));
	NS_LOG_INFO ("Run Simulation.");
	
  	Simulator::Run ();
  	Simulator::Destroy ();
	NS_LOG_INFO ("Done");  	
	return 0;
}





