#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
//#include "ns3/transparent-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BypassExample");

int main (int argc, char *argv[])
{
	LogComponentEnable("BypassExample",LOG_LEVEL_ALL);
	LogComponentEnable("OnOffApplication", LOG_LEVEL_ALL);
	LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
	NodeContainer c;
  	//NodeContainer c1;

	c.Create (3);
	//c1.Create (1);
	
	NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
	NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
	
	MFHelperA gstar;
  	Ipv4StaticRoutingHelper staticRouting;
	
  	Ipv4ListRoutingHelper list;

  	list.Add (staticRouting,10);
	list.Add (gstar,0);
	
	InternetStackHelper internet;
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	internet.Install (c);
	
	//Ipv4ListRoutingHelper list1;
	
  	//list1.Add (staticRouting,0);
	
	
	//InternetStackHelper internet1;
  	//internet1.SetRoutingHelper (list1); // has effect on the next Install ()
  	//internet1.Install (c1);
	
	// create the channels first without any IP addressing information
    NS_LOG_INFO ("Create channels.");
  	PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", StringValue ("54Mbps"));
	
  	NetDeviceContainer nd01 = p2p.Install (n01);
	NetDeviceContainer nd12 = p2p.Install (n12);
	
	// Later,add IP addresses.
	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	
  	Ipv4InterfaceContainer i01 = ipv4.Assign (nd01);
	ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);
	
	uint16_t port = 9;
	//BulkSendHelper		send ("ns3::UdpSocketFactory", InetSocketAddress (i01.GetAddress (0), port));
	//OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (i01.GetAddress (1), port));
	//onoff.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable(1000)));
	//onoff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0)));
	PacketSinkHelper	sink ("ns3::UdpSocketFactory",InetSocketAddress (i12.GetAddress (1), port));
	//TransparentHelper	transparent ("ns3::UdpSocketFactory",InetSocketAddress (i01.GetAddress(1), port), InetSocketAddress (i12.GetAddress(0), port));
	
	//ApplicationContainer apps = onoff.Install (c.Get (0));
	//apps.Start (Seconds (1.0));
  	//apps.Stop (Seconds (100.0));
	
	ApplicationContainer apps;
	apps = sink.Install (c.Get(2));
	apps.Start (Seconds (1.0));
  	apps.Stop (Seconds (100.0));
	
	//apps = transparent.Install (c.Get(1));
	//apps.Start (Seconds (1.0));
  	//apps.Stop (Seconds (100.0));
	
	Simulator::Stop (Seconds (110));
	NS_LOG_INFO ("Run Simulation.");
	
  	Simulator::Run ();
  	Simulator::Destroy ();
	NS_LOG_INFO ("Done");  	
	return 0;
	
	//Create the OnOff application to send UDP datagrams of size
  	// 210 bytes at a rate of 448 Kb/s from n0 to n1
  	////uint16_t port = 9;   // Discard port (RFC 863)
	
  	////OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (i01.GetAddress (1), port)); 
  	////onoff.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (10)));
  	////onoff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0)));
	
	////ApplicationContainer apps = onoff.Install (c.Get (0));
	////apps.Start (Seconds (1.0));
  	////apps.Stop (Seconds (10.0));
	
	// Create a packet sink to receive these packets
  	////PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
	
  	////apps = sink.Install (c.Get (1));
  	////apps.Start (Seconds (1.0));
  	////apps.Stop (Seconds (30.0));
	
	/*Simulator::Stop (Seconds (10));
	NS_LOG_INFO ("Run Simulation.");
	
  	Simulator::Run ();
  	Simulator::Destroy ();
	NS_LOG_INFO ("Done");  	
	return 0;*/
	
}