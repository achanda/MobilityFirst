#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpE2E");


// The number of bytes to send in this simulation.
static const uint32_t totalTxBytes = 2000000;
static uint32_t currentTxBytes = 0;
// Perform series of 1040 byte writes (this is a multiple of 26 since
// we want to detect data splicing in the output stream)
static const uint32_t writeSize = 1040;
uint8_t data[writeSize];

// These are for starting the writing process, and handling the sending 
// socket's notification upcalls (events).  These two together more or less
// implement a sending "Application", although not a proper ns3::Application
// subclass.

void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t);
void WriteUntilBufferFull (Ptr<Socket>, uint32_t);

static void 
CwndTracer (uint32_t oldval, uint32_t newval)
{
	NS_LOG_INFO ("Moving cwnd from " << oldval << " to " << newval);
}

int main (int argc, char *argv[])
{
	// Users may find it convenient to turn on explicit debugging
	// for selected modules; the below lines suggest how to do this
	//  LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
	//  LogComponentEnable("TcpSocketImpl", LOG_LEVEL_ALL);
	//  LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
	  LogComponentEnable("TcpE2E", LOG_LEVEL_ALL);
	
	CommandLine cmd;
	cmd.Parse (argc, argv);
	
	// initialize the tx buffer.
	for(uint32_t i = 0; i < writeSize; ++i)
    {
		char m = toascii (97 + i % 26);
		data[i] = m;
    }
	
	// Here, we will explicitly create 6 nodes.
	NodeContainer c;
	c.Create(5);
	
	NodeContainer n01 = NodeContainer (c.Get(0), c.Get(1));
	NodeContainer n12 = NodeContainer (c.Get(1), c.Get(2));
	NodeContainer n23 = NodeContainer (c.Get(2), c.Get(3));
	NodeContainer n34 = NodeContainer (c.Get(3), c.Get(4));
	
	// We create the channels first without any IP addressing information
	// First make and configure the helper, so that it will put the appropriate
	// attributes on the network interfaces and channels we are about to install.
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (10000000)));
	p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
	
	// And then install devices and channels connecting our topology.
	NetDeviceContainer dev0 = p2p.Install (n01);
	NetDeviceContainer dev1 = p2p.Install (n12);
	NetDeviceContainer dev2 = p2p.Install (n23);
	NetDeviceContainer dev3 = p2p.Install (n34);
	
	MobilityHelper mobility;
  	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  	positionAlloc->Add (Vector (10.0, 0.0, 0.0));
	positionAlloc->Add (Vector (20.0, 0.0, 0.0));
	positionAlloc->Add (Vector (30.0, 0.0, 0.0));
	positionAlloc->Add (Vector (40.0, 0.0, 0.0));
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
	
	// Now add ip/tcp stack to all nodes.
	InternetStackHelper internet;
	internet.InstallAll ();
	
	// Later, we add IP addresses.
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	ipv4.Assign (dev0);
	ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	ipv4.Assign (dev1);
	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	ipv4.Assign (dev2);
	ipv4.SetBase ("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer ipInterfs = ipv4.Assign (dev3);
	
	// and setup ip routing tables to get total ip-level connectivity.
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	///////////////////////////////////////////////////////////////////////////
	// Simulation 1
	//
	// Send 2000000 bytes over a connection to server port 50000 at time 0
	// Should observe SYN exchange, a lot of data segments and ACKS, and FIN 
	// exchange.  FIN exchange isn't quite compliant with TCP spec (see release
	// notes for more info)
	//
	///////////////////////////////////////////////////////////////////////////
	
	uint16_t servPort = 50000;
	
	// Create a packet sink to receive these packets on n2...
	PacketSinkHelper sink ("ns3::TcpSocketFactory",
						   InetSocketAddress (Ipv4Address::GetAny (), servPort));
	
	ApplicationContainer apps = sink.Install (n34.Get (1));
	apps.Start (Seconds (0.0));
	apps.Stop (Seconds (3.0));
	
	// Create a source to send packets from n0.  Instead of a full Application
	// and the helper APIs you might see in other example files, this example
	// will use sockets directly and register some socket callbacks as a sending
	// "Application".
	
	// Create and bind the socket...
	Ptr<Socket> localSocket =
    Socket::CreateSocket (n01.Get (0), TcpSocketFactory::GetTypeId ());
	localSocket->Bind ();
	
	// Trace changes to the congestion window
	Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
	
	// ...and schedule the sending "Application"; This is similar to what an 
	// ns3::Application subclass would do internally.
	Simulator::ScheduleNow (&StartFlow, localSocket,
							ipInterfs.GetAddress (1), servPort);
	
	// One can toggle the comment for the following line on or off to see the
	// effects of finite send buffer modelling.  One can also change the size of
	// said buffer.
	
	//localSocket->SetAttribute("SndBufSize", UintegerValue(4096));
	
	//Ask for ASCII and pcap traces of network traffic
	AsciiTraceHelper ascii;
	p2p.EnableAsciiAll (ascii.CreateFileStream ("tcp-e2e.tr"));
	p2p.EnablePcapAll ("tcp-e2e");
	
	// Finally, set up the simulator to run.  The 1000 second hard limit is a
	// failsafe in case some change above causes the simulation to never end
	Simulator::Stop (Seconds (1000));
	Simulator::Run ();
	Simulator::Destroy ();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//begin implementation of sending "Application"
void StartFlow (Ptr<Socket> localSocket,
                Ipv4Address servAddress,
                uint16_t servPort)
{
	NS_LOG_LOGIC ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
	localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect
	
	// tell the tcp implementation to call WriteUntilBufferFull again
	// if we blocked and new tx buffer space becomes available
	localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
	WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
	while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0) 
    {
		uint32_t left = totalTxBytes - currentTxBytes;
		uint32_t dataOffset = currentTxBytes % writeSize;
		uint32_t toWrite = writeSize - dataOffset;
		toWrite = std::min (toWrite, left);
		toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
		int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
		if(amountSent < 0)
        {
			// we will be called again when new tx space becomes available.
			return;
        }
		currentTxBytes += amountSent;
    }
	localSocket->Close ();
}
