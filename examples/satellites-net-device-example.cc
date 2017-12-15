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
#include "ns3/netanim-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3::SatellitesExample");

uint32_t satellites_amount = 3;
uint32_t ground_stations_amount = 0;
unsigned int    nodeNum = 0;
string mobilityTracePath;
string linksPath;

uint32_t maxBytes = 0;

vector<vector<vector<bool>>> loadLinks(string filename){
    int total_nodes = satellites_amount + ground_stations_amount;
    vector<vector<vector<bool>>> links;
    vector<vector<bool>> nodes;
    vector<bool> single_node;
    ifstream links_file;
    bool val;
    links_file.open(filename);
    while(links_file >> val) {
        for(int i = 0; i < total_nodes; i++) {
            for (int j = 0; j < total_nodes; j++) {
                links_file >> val;
                single_node.push_back(val);
            }
            nodes.push_back(single_node);
            single_node.clear();
        }
        links.push_back(nodes);
        nodes.clear();
    }
    cout << "Links size: " << links.size() << endl;
    links_file.close();
    return links;
}


int 
main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue ("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.AddValue ("tracePath", "Path to the NS2Mobility trace file", mobilityTracePath);
    cmd.AddValue ("linksPath", "Path to the links", linksPath);
    cmd.Parse (argc,argv);

    LogComponentEnable ("UdpEchoClientApplication", LOG_ALL);
    LogComponentEnable ("UdpEchoServerApplication", LOG_ALL);
    LogComponentEnable ("ns3::SatelliteNetDevice", LOG_ALL);
    LogComponentEnable ("ns3::SatelliteChannel", LOG_ALL);
    LogComponentEnable ("ns3::SatellitesExample", LOG_ALL);
    LogComponentEnable ("Ns2MobilityHelper", LOG_ALL);

	NS_LOG_INFO ("Objects creation");
	Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
	Ptr<Node> client = CreateObject<Node> ();
    Ptr<Node> client2 = CreateObject<Node> ();
    Ptr<Node> server = CreateObject<Node> ();
    Ptr<SatelliteNetDevice> clientNetDevice = CreateObject<SatelliteNetDevice> ();
    Ptr<SatelliteNetDevice> client2NetDevice = CreateObject<SatelliteNetDevice> ();
    Ptr<SatelliteNetDevice> serverNetDevice = CreateObject<SatelliteNetDevice> ();
	NodeContainer nodes;
	nodes.Add(client);
    nodes.Add(client2);
    nodes.Add(server);

	NS_LOG_INFO ("NetDevices setup");
    NetDeviceContainer netDevices;
    client->AddDevice(clientNetDevice);
    client->AddDevice(client2NetDevice);
    server->AddDevice(serverNetDevice);

    clientNetDevice->SetChannel(channel);
    client2NetDevice->SetChannel(channel);
    serverNetDevice->SetChannel(channel);

    clientNetDevice->SetDataRate(DataRate ("100MB/s"));
    client2NetDevice->SetDataRate(DataRate ("100MB/s"));
    serverNetDevice->SetDataRate(DataRate ("100MB/s"));

    Time time(0);
    clientNetDevice->SetInterframeGap(time);
    serverNetDevice->SetInterframeGap(time);
    clientNetDevice->SetInterframeGap(time);

    netDevices.Add(clientNetDevice);
	netDevices.Add(serverNetDevice);

	NS_LOG_INFO ("Channel setup");
    ObjectFactory m_propagationDelay;
    m_propagationDelay.SetTypeId("ns3::ConstantSpeedPropagationDelayModel");
    Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
    channel->SetPropagationDelay (delay);

    channel->Add(clientNetDevice);
    channel->Add(client2NetDevice);
    channel->Add(serverNetDevice);


    std::vector<std::vector<std::vector<bool>>> links;
    cout << linksPath;

    if(!linksPath.empty()) {
        links = loadLinks(linksPath);
        channel->SetLinks(links);
    }

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
    sourceApps.Stop (Seconds (500.0));


    PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install (nodes.Get (1));
    ApplicationContainer sinkApps2 = sink.Install (nodes.Get (2));

    sinkApps.Start (Seconds (0.0));
    sinkApps2.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (500.0));
    sinkApps2.Stop (Seconds (500.0));

    Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobilityTracePath);
    ns2.Install (); // configure movements for each node, while reading trace file

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(500));
    Simulator::Run ();

    flowMonitor->SerializeToXmlFile("flow.xml", true, true);

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

            std::cout << "  Throughput RX: " << i->second.rxBytes * 8 / 1000000 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds()) << " Mbits/s\n";
            std::cout << "  Throughput TX: " << i->second.txBytes * 8 / 1000000 / (i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) << " Mbits/s\n";

    }
    Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
	return 0;
}
