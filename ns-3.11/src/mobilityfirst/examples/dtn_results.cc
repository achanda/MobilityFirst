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
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleWifiAdhocExample");

#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
                     (time - Simulator::Now () + Seconds (0.000001)))

NodeContainer c;
NetDeviceContainer devices;
std::string phyMode("OfdmRate54Mbps");
int size = 7;
//double distance_x = 100.0;
void MoveNodeNear ();

void MoveNodeAway ()
{
	cout << "Moving Node Away" << endl;
	
	Ptr<Node> node_1 = c.Get (4);
  	Ptr<MobilityModel> mob_1 = node_1->GetObject<MobilityModel> ();
  	mob_1->SetPosition(Vector (800.0, 600.0, 0.0));

	Ptr<Node> node_2 = c.Get (6);
  	Ptr<MobilityModel> mob_2 = node_2->GetObject<MobilityModel> ();
  	mob_2->SetPosition(Vector (800.0, 800.0, 0.0));

	Time now = Simulator::Now ();
	Simulator::Schedule (Seconds (12.5), &MoveNodeNear);
}
	
void MoveNodeNear ()
{
	cout << "Moving Node Near" << endl;
	
	Ptr<Node> node_1 = c.Get (4);
  	Ptr<MobilityModel> mob_1 = node_1->GetObject<MobilityModel> ();
  	mob_1->SetPosition(Vector (400.0, 100.0, 0.0));

	Ptr<Node> node_2 = c.Get (6);
  	Ptr<MobilityModel> mob_2 = node_2->GetObject<MobilityModel> ();
  	mob_2->SetPosition(Vector (300.0, 200.0, 0.0));

	Time now = Simulator::Now ();
	Simulator::Schedule (Seconds (12.5), &MoveNodeAway);
}

int main (int argc, char *argv[])
{
	LogComponentEnable("SimpleWifiAdhocExample",LOG_LEVEL_ALL);
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      StringValue (phyMode));
	//double rss = -80;  // -dBm	
    
    uint32_t animPort = 0;
    std::string animFile;
    
    CommandLine cmd;
    cmd.AddValue ("animPort",      "Port Number for Remote Animation", animPort);
    cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);

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
	positionAlloc->Add (Vector (300.0, 100.0, 0.0));
	positionAlloc->Add (Vector (400.0, 100.0, 0.0));
	positionAlloc->Add (Vector (100.0, 0.0, 0.0));
	positionAlloc->Add (Vector (300.0, 200.0, 0.0));
	
	//positionAlloc->Add (Vector (200.0, 200.0, 0.0));
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
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "RtsCtsThreshold", UintegerValue (0));
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
  	
	Time now = Simulator::Now ();
	Simulator::Schedule (Seconds (27.5), &MoveNodeAway);

	Simulator::Stop (Seconds (90));
        NS_LOG_INFO ("Run Simulation.");
    
    AnimationInterface anim;
    if (animPort > 0)
    {
        anim.SetServerPort (animPort);
    }
    else if (!animFile.empty ())
    {
        anim.SetOutputFile (animFile);
    }
    anim.StartAnimation ();

  	Simulator::Run ();
  	Simulator::Destroy ();

        NS_LOG_INFO ("Done");  	
	return 0;
}

	



