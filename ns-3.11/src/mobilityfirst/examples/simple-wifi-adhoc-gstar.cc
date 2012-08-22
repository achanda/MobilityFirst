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

NodeContainer c;
NetDeviceContainer devices;
std::string phyMode("OfdmRate54Mbps");
int size = 2;
//double distance_x = 2.0;

void MoveNode ()
{
	cout << "Moving Node" << endl;
	Ptr<NetDevice> node = devices.Get(0);
	Ptr<WifiNetDevice> mob =  DynamicCast<WifiNetDevice> (node);
  	//Ptr<PointToPointNetDevice> mob = node->GetObject<PointToPointNetDevice> ();
        std::cout << "2" << std::endl;
	
	Ptr<WifiRemoteStationManager> station = mob->GetRemoteStationManager();
	mob->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate6Mbps"));
	mob->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate6Mbps"));
	Ptr<ConstantRateWifiManager> station_d =  DynamicCast<ConstantRateWifiManager> (station);
	Config::RegisterRootNamespaceObject(station_d);
	//uint32_t mode = station->GetNBasicModes();
	//cout << mode << endl;
	
	//phyMode = "OfdmRate6Mbps";
	//Config::SetDefault ("ns3::ConstantRateWifiManager::ControlMode", StringValue ("OfdmRate6Mbps"));
	//Config::Reset ();	
	
	//Ptr<ConstantRateWifiManager> delayModel = CreateObject<ConstantRateWifiManager> ();
	//Config::SetDefault ("ns3::ConstantRateWifiManager::ControlMode", StringValue ("OfdmRate6Mbps"));
	//Config::SetDefault ("ns3::ConstantRateWifiManager::DataMode", StringValue ("OfdmRate6Mbps"));
	//Ptr<WifiRemoteStationManager> delay =  DynamicCast<WifiRemoteStationManager> (delayModel);

	
	cout << "3" << endl;
	
	//WifiMode test_mode ("OfdmRate9Mbps");
	//WifiMode& blah = test_mode;
	//Config::reset ();
	cout << "4" << endl;
	//mob->SetRemoteStationManager(delay);
	
	Ptr<NetDevice> node_1 = devices.Get(1);
	Ptr<WifiNetDevice> mob_1 =  DynamicCast<WifiNetDevice> (node_1);
	
  	//Ptr<PointToPointNetDevice> mob = node->GetObject<PointToPointNetDevice> ();
        //std::cout << "2" << std::endl;
	//phyMode = "OfdmRate6Mbps";
	Ptr<WifiRemoteStationManager> station_1 = mob_1->GetRemoteStationManager();
	cout << station_1 -> GetNonUnicastMode() << endl;
	mob_1->GetRemoteStationManager()->SetAttribute ("DataMode", StringValue ("OfdmRate6Mbps"));
	mob_1->GetRemoteStationManager()->SetAttribute ("ControlMode", StringValue ("OfdmRate6Mbps"));
	//Ptr<ConstantRateWifiManager> station_d1 =  DynamicCast<ConstantRateWifiManager> (station_1);
	//Config::RegisterRootNamespaceObject(station_d1);
	
	//WifiMode mode = station_1->GetDefaultMode();
	//cout << mode << endl;
	//WifiMode mode_1 = station->GetBasicMode(0);
	//Ptr<ConstantRateWifiManager> delayModel_1 = CreateObject<ConstantRateWifiManager> ();
	//Config::SetDefault ("ns3::ConstantRateWifiManager::ControlMode", StringValue ("DsssRate1Mbps"));
	//Config::SetDefault ("ns3::ConstantRateWifiManager::DataMode", StringValue ("DsssRate1Mbps"));
	//Ptr<WifiRemoteStationManager> delay_1 =  DynamicCast<WifiRemoteStationManager> (delayModel_1);
	//mob_1->SetRemoteStationManager(delay_1);
	
	//cout << mode_1 << endl;
	//Ptr<Node> node = c.Get (1);
  	//Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  	//mob->SetPosition(Vector (1000, 0.0, 0.0));
	//cout << mob->GetPosition() << endl;
	//Ptr<Node> node_0 = c.Get (0);
  	//Ptr<MobilityModel> mob_0 = node_0->GetObject<MobilityModel> ();
	//cout << mob_0->GetPosition() << endl;
}
	
/*void MoveNode ()
{
	cout << "Moving Node" << endl;
	//Ptr<Node> node = c.Get (0);
	Ptr<NetDevice> node = devices.Get(0);
  	Ptr<WifiHelper> mob = node->GetObject<WifiHelper> ();
  	//Simulator::Schedule (Seconds (10), &MobilityModel::SetPosition, mob, Vector (0.0, 100.0, 0.0));
}*/

int main (int argc, char *argv[])
{
	LogComponentEnable("SimpleWifiAdhocExample",LOG_LEVEL_ALL);
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
  	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  	positionAlloc->Add (Vector (25.0, 0.0, 0.0));
	//positionAlloc->Add (Vector (200.0, 0.0, 0.0));
	//positionAlloc->Add (Vector (100.0, 0.0, 0.0));
 	mobility.SetPositionAllocator (positionAlloc);
  	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility.Install (c);

	//MobilityHelper mobility;
  	//mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
         //                        "MinX", DoubleValue (0.0),
          //                       "MinY", DoubleValue (0.0),
           //                      "DeltaX", DoubleValue (distance_x),
             //                    "DeltaY", DoubleValue (0),
               //                  "GridWidth", UintegerValue (size),
                 //                "LayoutType", StringValue ("RowFirst"));
  	//mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	//mobility.Install (c);

	//MobilityHelper mobility;
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
	//wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	
	//wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	//wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  	wifiPhy.SetChannel (wifiChannel.Create ());
  	WifiHelper wifi = WifiHelper::Default ();
  	
	//wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",  "ControlMode", StringValue (phyMode), "DataMode", StringValue (phyMode));
  	//wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"),"RtsCtsThreshold", UintegerValue (0));
	//wifi.SetRemoteStationManager ("ns3::RraaWifiManager","RtsCtsThreshold", UintegerValue (0));
  	devices = wifi.Install (wifiPhy, wifiMac, c); 
	
	 // Enable GSTAR
    	NS_LOG_INFO ("Enabling GSTAR Routing.");
  	MFHelper gstar;

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
  	
	Simulator::Schedule (Seconds (50), &MoveNode);

	Simulator::Stop (Seconds (100));
        NS_LOG_INFO ("Run Simulation.");

  	Simulator::Run ();
  	Simulator::Destroy ();
        NS_LOG_INFO ("Done");  	
	return 0;
}

	



