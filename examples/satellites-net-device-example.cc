#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>
#include <ns3/mobility-module.h>
#include <ns3/bulk-send-helper.h>
#include <ns3/packet-sink.h>
#include <ns3/packet-sink-helper.h>
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3::SatellitesExample");

uint32_t satellites_amount = 0;
uint32_t ground_stations_amount = 0;
unsigned int    nodeNum = 0;
string mobilityTracePath;
uint32_t maxBytes = 0;

int 
main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue ("tracePath", "Path to the NS2Mobility trace file", mobilityTracePath);
    cmd.AddValue ("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.Parse (argc,argv);
    cout << mobilityTracePath << endl;

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

    clientNetDevice->SetDataRate(DataRate ("100MB/s"));
    serverNetDevice->SetDataRate(DataRate ("100MB/s"));

    Time time(0);
    clientNetDevice->SetInterframeGap(time);
    serverNetDevice->SetInterframeGap(time);

    netDevices.Add(clientNetDevice);
	netDevices.Add(serverNetDevice);

	NS_LOG_INFO ("Channel setup");
    ObjectFactory m_propagationDelay;
    m_propagationDelay.SetTypeId("ns3::ConstantSpeedPropagationDelayModel");
    Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
    channel->SetPropagationDelay (delay);

    channel->Add(clientNetDevice);
    channel->Add(serverNetDevice);


    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (netDevices);

    uint16_t port = 9;
    BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (1), port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    ApplicationContainer sourceApps = source.Install (nodes.Get (0));
    sourceApps.Start (Seconds (0.0));
    sourceApps.Stop (Seconds (200.0));


    PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install (nodes.Get (1));
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (200.0));

    Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobilityTracePath);
    ns2.Install (); // configure movements for each node, while reading trace file

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    std::cout << "RUN" << std::endl;
    Simulator::Stop(Seconds(210));
    Simulator::Run ();

    flowMonitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
            std::cout << "Flow " << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput RX: " << i->second.rxBytes * 8 / 1024 / 200 << " Megabits/s\n";
            std::cout << "  Throughput TX: " << i->second.txBytes * 8 / 1024 / 200 << " Megabits/s\n";
    }


    Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
	return 0;
}
