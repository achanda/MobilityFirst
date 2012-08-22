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

#include "ns3/mf-helper.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleWifiAdhocExample");

#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
                     (time - Simulator::Now () + Seconds (0.000001)))

NodeContainer c;
NetDeviceContainer devices;
std::string phyMode("OfdmRate54Mbps");
int size = 8;
//double distapositionAlloc->Add (Vector (400.0, 200.0, 0.0));nce_x = 100.0;
void MoveNodeNear ();

void MoveNodeAway ()
{
	cout << "Moving Node Away" << endl;
	//Ptr<NetDevice> node = devices.Get(0);
	//Ptr<WifiNetDevice> mob =  DynamicCast<WifiNetDevice> (node);
  	
	//Ptr<WifiRemoteStationManager> station = mob->GetRemoteStationManager();
	//mob->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate6Mbps"));
	//mob->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate6Mbps"));
	
	Ptr<NetDevice> node_1 = devices.Get(3);
	Ptr<WifiNetDevice> mob_1 =  DynamicCast<WifiNetDevice> (node_1);

	mob_1->GetPhy()->SetAttribute ("TxPowerStart",DoubleValue(17));
	mob_1->GetPhy()->SetAttribute ("TxPowerEnd",DoubleValue(17));
	
	
  	Ptr<WifiRemoteStationManager> station_1 = mob_1->GetRemoteStationManager();
	mob_1->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate6Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate6Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("NonUnicastMode", StringValue ("OfdmRate6Mbps"));

	Time now = Simulator::Now ();
	Simulator::Schedule (Seconds (5), &MoveNodeNear);
}
	
void MoveNodeNear ()
{
	cout << "Moving Node Near" << endl;
	//Ptr<NetDevice> node = devices.Get(0);
	//Ptr<WifiNetDevice> mob =  DynamicCast<WifiNetDevice> (node);
  	
	//Ptr<WifiRemoteStationManager> station = mob->GetRemoteStationManager();
	//mob->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate54Mbps"));
	//mob->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate54Mbps"));
	
	Ptr<NetDevice> node_1 = devices.Get(3);
	Ptr<WifiNetDevice> mob_1 =  DynamicCast<WifiNetDevice> (node_1);

	mob_1->GetPhy()->SetAttribute ("TxPowerStart",DoubleValue(35));
	mob_1->GetPhy()->SetAttribute ("TxPowerEnd",DoubleValue(35));
	
  	Ptr<WifiRemoteStationManager> station_1 = mob_1->GetRemoteStationManager();
	mob_1->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate54Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate54Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("NonUnicastMode", StringValue ("OfdmRate54Mbps"));

	Time now = Simulator::Now ();
	Simulator::Schedule (Seconds (5), &MoveNodeAway);
}

int main (int argc, char *argv[])
{
	LogComponentEnable("SimpleWifiAdhocExample",LOG_LEVEL_ALL);
	//Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                    //  StringValue (phyMode));
	//Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("220"));
	//double rss = -80;  // -dBm
	
	CommandLine cmd;

	//cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
	//Config::Set ("ns3::ConstantRateWifiManager::ControlMode", StringValue ("OfdmRate12Mbps"));
	//cmd.AddValue ("rss", "received signal strength", rss);
	cmd.Parse (argc, argv);

	NS_LOG_INFO("Create nodes");
	NodeContainer ny;
  	c.Create (size);	  
	
  	MobilityHelper mobility;
  	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	//positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	//positionAlloc->Add (Vector (0.0, 50.0, 0.0));
	//positionAlloc->Add (Vector (0.0, 100.0, 0.0));
  	positionAlloc->Add (Vector (0.0, 100.0, 0.0));
  	positionAlloc->Add (Vector (100.0, 100.0, 0.0));
	positionAlloc->Add (Vector (200.0, 100.0, 0.0));
	positionAlloc->Add (Vector (200.0, 200.0, 0.0));
	positionAlloc->Add (Vector (200.0, 0.0, 0.0));
	positionAlloc->Add (Vector (300.0, 200.0, 0.0));
	positionAlloc->Add (Vector (300.0, 0.0, 0.0));
	positionAlloc->Add (Vector (100.0, 0.0, 0.0));
	
	//positionAlloc->Add (Vector (200.0, 0.0, 0.0));
	//positionAlloc->Add (Vector (100.0, 0.0, 0.0));
 	mobility.SetPositionAllocator (positionAlloc);
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (c);

	//MobilityHelper mobility;
  	/*mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance_x),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (size),
                                 "LayoutType", StringValue ("RowFirst"));
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (c);*/

	//MobilityHelper mobility;#include <nqos-wifi-mac-helper.h>
  	//.SetPositionAllocator ("ns3::GridPositionAllocator",
        //	                       "MinX", DoubleValue (0.0),
          //                             "MinY", DoubleValue (0.0),
            //                           "DeltaX", DoubleValue (100),
              //                         "DeltaY", DoubleValue (0),
                //                       "GridWidth", UintegerValue (2),
                  ///                     "LayoutType", StringValue ("RowFirst"));

	//Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  	//positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  	//positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  	//mobility.SetPositionAllocator (positionAlloc);
  	//mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	//mobility.Install (c);

	
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  	wifiMac.SetType ("ns3::AdhocWifiMac");
  	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	wifiPhy.Set("TxPowerStart",DoubleValue(35)); // 250-300 meter transmission range 
  	wifiPhy.Set("TxPowerEnd",DoubleValue(35));   // 250-300 meter transmission range 

	//Ptr<FriisPropagationLossModel > lossModel = CreateObject<FriisPropagationLossModel> ();
	//Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
	

  	////lossModel->SetDefaultLoss (0); // set loss to 0 dB 


	//Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();

  	//wifiChannel->SetPropagationLossModel (lossModel);
	//wifiChannel->SetPropagationDelayModel (delayModel);

  	//wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

	//wifiPhy.Set ("RxGain", DoubleValue (0) ); 
  	////YansWifiChannelHelper wifiChannel;
	////wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	
	////wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	//wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  	//wifiPhy.SetChannel (wifiChannel);
  	//WifiHelper wifi;
	

	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	
	//wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	//wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  	wifiPhy.SetChannel (wifiChannel.Create ());
  	WifiHelper wifi = WifiHelper::Default ();
  	
	//wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "RtsCtsThreshold", UintegerValue (0), "NonUnicastMode", StringValue (phyMode));
  	//wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"),"RtsCtsThreshold", UintegerValue (0));
	//wifi.SetRemoteStationManager ("ns3::RraaWifiManager","RtsCtsThreshold", UintegerValue (0));
  	devices = wifi.Install (wifiPhy, wifiMac, c); 
	
	 // Enable GSTAR
    	NS_LOG_INFO ("Enabling GSTAR Routing.");
  	MFHelper gstar;
	
	gstar.SetAttribute("Nodes", UintegerValue(100));
	gstar.SetAttribute("Flows", UintegerValue(2));
	gstar.SetAttribute("Seed", UintegerValue(2));

  	Ipv4StaticRoutingHelper staticRouting;

  	Ipv4ListRoutingHelper list;

  	list.Add (staticRouting, 10);
  	list.Add (gstar, 0);

  	InternetStackHelper internet;
  	internet.SetRoutingHelper (list); // has effect on the next Install ()
  	internet.Install (c);
	
	// Later, add IP addresses.
	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  	Ipv4InterfaceContainer i01 = ipv4.Assign (devices);
	

	 // move node away
  	
	Time now = Simulator::Now ();
	//Simulator::Schedule (Seconds (5), &MoveNodeAway);

	Simulator::Stop (Seconds (10));
        NS_LOG_INFO ("Run Simulation.");

  	Simulator::Run ();
  	Simulator::Destroy ();
        NS_LOG_INFO ("Done");  	
	return 0;
}

	



