#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>
#include <ns3/mobility-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/udp-echo-helper.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3::SatellitesExample");

uint32_t satellites_amount = 0;
uint32_t ground_stations_amount = 0;
unsigned int    nodeNum = 0;
double duration;

int 
main(int argc, char *argv[]) {

    Time::SetResolution (Time::S);
    LogComponentEnable ("UdpEchoClientApplication", LOG_ALL);
    LogComponentEnable ("UdpEchoServerApplication", LOG_ALL);
    LogComponentEnable ("ns3::SatelliteNetDevice", LOG_ALL);
    LogComponentEnable ("ns3::SatelliteChannel", LOG_ALL);
    LogComponentEnable ("ns3::SatellitesExample", LOG_ALL);
    LogComponentEnable ("Ns2MobilityHelper", LOG_ALL);

	NS_LOG_INFO ("Objects creation");
	Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
	Ptr<Node> client = CreateObject<Node> ();
	Ptr<Node> server = CreateObject<Node> ();
	Ptr<SatelliteNetDevice> clientNetDevice = CreateObject<SatelliteNetDevice> ();
	Ptr<SatelliteNetDevice> serverNetDevice = CreateObject<SatelliteNetDevice> ();
	NodeContainer nodes;
	nodes.Add(client);
	nodes.Add(server);

	NS_LOG_INFO ("NetDevices setup");
    NetDeviceContainer netDevices;
    client->AddDevice(clientNetDevice);
    server->AddDevice(serverNetDevice);
    clientNetDevice->SetChannel(channel);
    serverNetDevice->SetChannel(channel);
	netDevices.Add(clientNetDevice);
	netDevices.Add(serverNetDevice);

	NS_LOG_INFO ("Channel setup");
	channel->Add(clientNetDevice);
    channel->Add(serverNetDevice);


    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (netDevices);

    UdpEchoServerHelper echoServer (9);

    ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue ("Time"),
                              "Time", StringValue ("2s"),
                              "Speed",
                              StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds", RectangleValue (Rectangle (0.0, 20.0, 0.0, 20.0)));
    mobility.Install(nodes);

    Simulator::Run ();
    Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
	return 0;
}
