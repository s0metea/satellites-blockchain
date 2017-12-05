#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3::SatellitesExample");

int 
main(int argc, char *argv[]) {
	cout << "Test!\n";
	NS_LOG_INFO ("Objects creation");
	Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
	Ptr<Node> client = CreateObject<Node> ();
	Ptr<Node> server = CreateObject<Node> ();
	Ptr<SatelliteNetDevice> clientNetDevice = CreateObject<SatelliteNetDevice> ();
	Ptr<SatelliteNetDevice> serverNetDevice = CreateObject<SatelliteNetDevice> ();
	NetDeviceContainer netDevices;
	NodeContainer nodes;
	nodes.Add(client);
	nodes.Add(server);

	NS_LOG_INFO ("NetDevices setup");
    client->AddDevice(clientNetDevice);
    server->AddDevice(serverNetDevice);
	netDevices.Add(clientNetDevice);
	netDevices.Add(serverNetDevice);

	NS_LOG_INFO ("Channel setup");
	channel->Add(clientNetDevice);
    channel->Add(serverNetDevice);

    NS_LOG_INFO ("Assign IP Addresses.");
    InternetStackHelper internet;
	Ipv4AddressHelper ipv4;
    internet.Install(nodes);
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign (netDevices);
	Address serverAddress = Address(i.GetAddress (1));

	NS_LOG_INFO ("Create Application.");
    // Create udpServer application on server:
	uint16_t port = 4000;
	UdpServerHelper serverHelper (port);
	ApplicationContainer server_app = serverHelper.Install (server);
	server_app.Start (Seconds (1.0));
	server_app.Stop (Seconds (10.0));
	//
	// Create one UdpClient application to send UDP datagrams from node zero to
	// node one.
	uint32_t MaxPacketSize = 1024;
	uint32_t maxPacketCount = 320;
    Time interPacketInterval = Seconds (0.05);
    UdpClientHelper clientHelper (serverAddress, port);
	clientHelper.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
	clientHelper.SetAttribute ("Interval", TimeValue (interPacketInterval));
	clientHelper.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
    ApplicationContainer client_app = clientHelper.Install (client);
	client_app.Start (Seconds (2.0));
	client_app.Stop (Seconds (10.0));
	NS_LOG_INFO ("Run Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
	return 0;
}
