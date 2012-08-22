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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimplePointToPointGstarExample");

NodeContainer c;
NetDeviceContainer nd01;

void MoveNode ()
{
        //DataRate x("54000bps");

        //uint64_t bit = 11000;

	std::cout << "Moving Node" << std::endl;
	Ptr<NetDevice> node = nd01.Get (0);
        Ptr<PointToPointNetDevice> mob =  DynamicCast<PointToPointNetDevice> (node);
  	//Ptr<PointToPointNetDevice> mob = node->GetObject<PointToPointNetDevice> ();
        std::cout << "2" << std::endl;
        //Ptr<PointToPointNetDevice> net = mob->GetPointToPointDevice(1);
        //Ptr<Node> node_1 = node->GetNode();
        //mob->SetDeviceAttribute ("DataRate", StringValue ("11Mbps"));
        mob->SetDataRate (11000000);
        uint16_t test = node->GetMtu();
        std::cout << "3" << test << std::endl;
}

int main (int argc, char *argv[])
{
        LogComponentEnable("SimplePointToPointGstarExample",LOG_LEVEL_ALL);
	Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  	Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

	CommandLine cmd;
  	cmd.Parse (argc, argv);

        NS_LOG_INFO("Create nodes");
	
  	c.Create (2);
  	
        NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
        ////NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
        ////NodeContainer n23 = NodeContainer (c.Get (2), c.Get (3));
        ////NodeContainer n34 = NodeContainer (c.Get (3), c.Get (4));
        ////NodeContainer n45 = NodeContainer (c.Get (4), c.Get (5));
        ////NodeContainer n17 = NodeContainer (c.Get (1), c.Get (7));
        ////NodeContainer n46 = NodeContainer (c.Get (4), c.Get (6));
        
        // Enable GSTAR
        NS_LOG_INFO ("Enabling GSTAR Routing.");
  	MFHelper gstar;

  	Ipv4StaticRoutingHelper staticRouting;

  	Ipv4ListRoutingHelper list;

  	list.Add (staticRouting,10);
  	list.Add (gstar, 0);

  	InternetStackHelper internet;
       
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	internet.Install (c);

	// create the channels first without any IP addressing information
    NS_LOG_INFO ("Create channels.");
  	PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", StringValue ("54Mbps"));
  	//p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
        

  	nd01 = p2p.Install (n01);
        ////NetDeviceContainer nd12 = p2p.Install (n12);
        ////NetDeviceContainer nd23 = p2p.Install (n23);
        ////NetDeviceContainer nd34 = p2p.Install (n34);
        ////NetDeviceContainer nd45 = p2p.Install (n45);
        ////NetDeviceContainer nd17 = p2p.Install (n17);
        ////NetDeviceContainer nd46 = p2p.Install (n46);

        //p2p.SetDeviceAttribute ("DataRate", StringValue ("11Mbps"));
        
        //nd01.Add(dv1);



	// Later,add IP addresses.
        NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
        
        
  	Ipv4InterfaceContainer i01 = ipv4.Assign (nd01);
        
        ///ipv4.SetBase ("10.1.2.0", "255.255.255.0");
        ////Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);
        
        ////ipv4.SetBase ("10.1.3.0", "255.255.255.0");
        ////Ipv4InterfaceContainer i23 = ipv4.Assign (nd23); 
        ////ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  	////Ipv4InterfaceContainer i34 = ipv4.Assign (nd34);
        ////ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  	////Ipv4InterfaceContainer i45 = ipv4.Assign (nd45);
        ////ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  	////Ipv4InterfaceContainer i17 = ipv4.Assign (nd17);
        ////ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  	////Ipv4InterfaceContainer i46 = ipv4.Assign (nd46);

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

        //Simulator::Schedule (Seconds (40), &MoveNode);

	Simulator::Stop (Seconds (80));
        NS_LOG_INFO ("Run Simulation.");

  	Simulator::Run ();
  	Simulator::Destroy ();
        NS_LOG_INFO ("Done");  	
	return 0;
}

	



