/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BypassExample");

int main (int argc, char *argv[])
{   
	LogComponentEnable("BypassExample",LOG_LEVEL_ALL);
	uint32_t maxBytes = 100000;

	NS_LOG_INFO("Create nodes");
	
	NodeContainer c;
  	c.Create (7);
  	
	NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
	NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
	NodeContainer n23 = NodeContainer (c.Get (2), c.Get (3));
	NodeContainer n34 = NodeContainer (c.Get (3), c.Get (4));
	NodeContainer n46 = NodeContainer (c.Get (4), c.Get (5), c.Get (6));

        InternetStackHelper internet;
	internet.Install (c);

	NS_LOG_INFO ("Create channels.");
  	PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", StringValue ("54Mbps"));	
	
	NetDeviceContainer nd12 = p2p.Install (n12);
	NetDeviceContainer nd23 = p2p.Install (n23);
	NetDeviceContainer nd34 = p2p.Install (n34);
	
	MobilityHelper mobility;
  	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  	positionAlloc->Add (Vector (10.0, 0.0, 0.0));
	
 	mobility.SetPositionAllocator (positionAlloc);
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (n01);
		//mobility.Install (n01);
	
	MobilityHelper mobilityn;
  	Ptr<ListPositionAllocator> positionAllocn = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector  (40.0, 0.0, 0.0));
  	positionAllocn->Add (Vector (50.0, 0.0, 0.0));
	positionAllocn->Add (Vector (60.0, 0.0, 0.0));

 	mobilityn.SetPositionAllocator (positionAllocn);
  	mobilityn.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobilityn.Install (n46);
	
	NetDeviceContainer devices;
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  	wifiMac.SetType ("ns3::AdhocWifiMac");
  	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  	wifiPhy.SetChannel (wifiChannel.Create ());
  	WifiHelper wifi = WifiHelper::Default ();
  	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"),"RtsCtsThreshold", UintegerValue (0));
  	devices = wifi.Install (wifiPhy, wifiMac, n01);
	
	NetDeviceContainer devicesn;
  	devicesn = wifi.Install (wifiPhy, wifiMac, n46);
        
	
	Ipv4AddressHelper ipv4;

	ipv4.SetBase ("192.168.1.0", "255.255.0.0");
	Ipv4InterfaceContainer idevices = ipv4.Assign (devices);
	
	ipv4.SetBase ("192.168.2.0", "255.255.0.0");
	Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);
	
	ipv4.SetBase ("192.168.3.0", "255.255.0.0");
	Ipv4InterfaceContainer i23 = ipv4.Assign (nd23);
	ipv4.SetBase ("192.168.4.0", "255.255.0.0");
  	Ipv4InterfaceContainer i34 = ipv4.Assign (nd34);
	ipv4.SetBase ("192.168.5.0", "255.255.0.0");
  	Ipv4InterfaceContainer idevicesn = ipv4.Assign (devicesn);

	// Create router nodes, initialize routing database and set up the routing
  	// tables in the nodes.
  	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	 NS_LOG_INFO ("Create Applications.");

	//	
	// Create a BulkSendApplication and install it on node 0
	//
  	uint16_t port = 9;  // well-known echo port number


  	BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (idevicesn.GetAddress (2), port));
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

	flowmon->SerializeToXmlFile ("simple-mixed-routing.flowmon", false, false);

  	Simulator::Destroy ();

	NS_LOG_INFO ("Done");  	
	return 0;
}

