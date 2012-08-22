#include <iostream>
#include <fstream>
#include <string>
#include <cassert>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/config-store-module.h"

#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/ideal-wifi-manager.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/wifi-mode.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/data-rate.h"

#include "ns3/mf-helper.h"



using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("SimpleWifiAdhocExample");

#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
                     (time - Simulator::Now () + Seconds (0.000001)))

NodeContainer c;
NetDeviceContainer devices;
std::string phyMode("OfdmRate54Mbps");
int size = 9; //create 9 nodes

void MoveNodeNear ();

void MoveNodeAway ()
{
	cout << "Moving Node Away" << endl;

	
	//Ptr<Node> node_1 = c.Get(7);
	Ptr<NetDevice> node_1 = devices.Get(7);
	

	//Ptr<MobilityModel> mob_1 = node_1->GetObject<MobilityModel> ();
  	//mob_1->SetPosition(Vector (900.0, 900.0, 0.0));

	Ptr<WifiNetDevice> mob_1 =  DynamicCast<WifiNetDevice> (node_1);
	mob_1->GetPhy()->SetAttribute ("TxPowerStart",DoubleValue(17));
	mob_1->GetPhy()->SetAttribute ("TxPowerEnd",DoubleValue(17));
	
	
  	Ptr<WifiRemoteStationManager> station_1 = mob_1->GetRemoteStationManager();
	mob_1->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate6Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate6Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("NonUnicastMode", StringValue ("OfdmRate6Mbps"));

	

	
	Simulator::Schedule (Seconds (5), &MoveNodeNear);
}

void MoveNodeNear ()
{
	cout << "Moving Node Near" << endl;
	
	
	//Ptr<Node> node_1 = c.Get(7);
	Ptr<NetDevice> node_1 = devices.Get(7);

	//Ptr<MobilityModel> mob_1 = node_1->GetObject<MobilityModel> ();
  	//mob_1->SetPosition(Vector (300.0, 200.0, 0.0));

	Ptr<WifiNetDevice> mob_1 =  DynamicCast<WifiNetDevice> (node_1);

	mob_1->GetPhy()->SetAttribute ("TxPowerStart",DoubleValue(35));
	mob_1->GetPhy()->SetAttribute ("TxPowerEnd",DoubleValue(35));
	
  	Ptr<WifiRemoteStationManager> station_1 = mob_1->GetRemoteStationManager();
	mob_1->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate54Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate54Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("NonUnicastMode", StringValue ("OfdmRate54Mbps"));

	
	Simulator::Schedule (Seconds (5), &MoveNodeAway);
}

int main (int argc, char *argv[])
{
	int nflows=2;
	LogComponentEnable("SimpleWifiAdhocExample",LOG_LEVEL_ALL);

	Config::SetDefault ("ns3::mf::RoutingProtocol::Nodes", UintegerValue(size));
	Config::SetDefault ("ns3::mf::RoutingProtocol::Flows", UintegerValue(nflows));
	
	CommandLine cmd;
	cmd.Parse (argc, argv);
 	
	NS_LOG_INFO("Create nodes");
	//NodeContainer ny;
  	c.Create (size);	  

	
  	MobilityHelper mobility;

	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  	positionAlloc->Add (Vector (0.0, 200.0, 0.0));
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));			
	positionAlloc->Add (Vector (0.0, 100.0, 0.0));
	positionAlloc->Add (Vector (100.0, 100.0, 0.0));
	positionAlloc->Add (Vector (200.0, 100.0, 0.0));
	positionAlloc->Add (Vector (200.0, 200.0, 0.0));
	positionAlloc->Add (Vector (200.0, 0.0, 0.0));
	positionAlloc->Add (Vector (300.0, 200.0, 0.0));
	positionAlloc->Add (Vector (300.0, 0.0, 0.0));
	mobility.SetPositionAllocator (positionAlloc);
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (c);


	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  	wifiMac.SetType ("ns3::AdhocWifiMac");
  	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	wifiPhy.Set("TxPowerStart",DoubleValue(35)); // 250-300 meter transmission range 
  	wifiPhy.Set("TxPowerEnd",DoubleValue(35));   // 250-300 meter transmission range 
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	
	
  	wifiPhy.SetChannel (wifiChannel.Create ());
  	WifiHelper wifi = WifiHelper::Default ();
  	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "RtsCtsThreshold", UintegerValue (0), "NonUnicastMode", StringValue (phyMode));
  	
	devices = wifi.Install (wifiPhy, wifiMac, c);
  	 
	
	 // Enable GSTAR
    	NS_LOG_INFO ("Enabling GSTAR Routing.");
  	MFHelper gstar;
	
	//gstar.SetAttribute("Nodes", UintegerValue(100));
	//gstar.SetAttribute("Flows", UintegerValue(2));
	//gstar.SetAttribute("Seed", UintegerValue(2));

  	Ipv4StaticRoutingHelper staticRouting;

  	Ipv4ListRoutingHelper list;

  	list.Add (staticRouting, 10);
  	list.Add (gstar, 0);

  	InternetStackHelper internet;
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	internet.Install (c);
	
	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  	Ipv4InterfaceContainer i01 = ipv4.Assign (devices);
	//NodeContainer n02 = NodeContainer (c.Get (0), c.Get (2));
	

	

	//Create the OnOff application to send UDP datagrams of size
  	// 210 bytes at a rate of 448 Kb/s from n0 to n1
  	uint16_t port = 9;   // Discard port (RFC 863)

        OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (i01.GetAddress(1), port)); 
	//cout<<"Yeah!!"<<endl;
  	onoff.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (10)));
  	onoff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0)));
	onoff.SetAttribute("PacketSize",UintegerValue (5)); 

        ApplicationContainer apps = onoff.Install (c.Get (0));
        apps.Start (Seconds (11.0));
	//cout<<"Sending Data"<<endl;
  	apps.Stop (Seconds (20.0));
	//cout<<"Source stopped"<<endl;

	/*OnOffHelper onoff2 ("ns3::UdpSocketFactory", InetSocketAddress (i01.GetAddress(2,0), port)); 
	
  	onoff2.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (10)));
  	onoff2.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0)));
	onoff2.SetAttribute("PacketSize",UintegerValue (5)); 
	ApplicationContainer apps2 = onoff2.Install (c.Get (1));
        apps2.Start (Seconds (11.0));
  	apps2.Stop (Seconds (20.0));*/

	 // Create a packet sink to receive these packets
  	PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));

  	apps = sink.Install (c.Get (7));
  	apps.Start (Seconds (11.0));
	//cout<<"Sink started"<<endl;
  	apps.Stop (Seconds (20.0));
	//cout<<"Sink Destroyed"<<endl;

	/*PacketSinkHelper sink2 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
	apps2 = sink2.Install (c.Get (8));
  	apps2.Start (Seconds (11.0));
  	apps2.Stop (Seconds (20.0)); */ 

	//Simulator::Schedule (Seconds (5), &MoveNodeAway);

	Simulator::Stop (Seconds (50));
        NS_LOG_INFO ("Run Simulation.");

	AsciiTraceHelper ascii;
	wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("myfirst.tr"));
	wifiPhy.EnablePcapAll ("myfirst");
  	Simulator::Run ();
  	Simulator::Destroy ();
        NS_LOG_INFO ("Done");  	
	return 0;
}
