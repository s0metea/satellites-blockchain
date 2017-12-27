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
#include <ns3/low-resolution-radio-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/v4ping-helper.h>
#include <ns3/qemunet-module.h>
#include <ns3/satellites-blockchain-helper.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3::SatellitesExample");

vector<vector<vector<bool>>> loadLinks(string filename){
    int total_nodes = 3;
    vector<vector<vector<bool>>> links;
    vector<vector<bool>> nodes;
    vector<bool> single_node;
    ifstream links_file;
    bool val;
    links_file.open(filename.c_str(), ios_base::in);
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
	string mobilityTracePath;
	string linksPath;
	string linksTimeFrame;
	uint32_t maxBytes = 0;

    CommandLine cmd;
    cmd.AddValue ("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.AddValue ("tracePath", "Path to the NS2Mobility trace file", mobilityTracePath);
    cmd.AddValue ("linksPath", "Path to the links", linksPath);
    cmd.AddValue("linksTimeFrame", "Links time frame", linksTimeFrame);
    cmd.Parse (argc,argv);

    LogComponentEnable ("ns3::SatelliteNetDevice", LOG_ALL);
    LogComponentEnable ("ns3::SatelliteChannel", LOG_ALL);
    LogComponentEnable ("ns3::SatellitesExample", LOG_ALL);
    LogComponentEnable ("Ns2MobilityHelper", LOG_ALL);

	NS_LOG_INFO ("Objects creation");
	NodeContainer nodes;
	SatellitesHelper satHelper;
    nodes = satHelper.ConfigureNodes(3, DataRate ("100MB/s"), Time(0));

    NS_LOG_INFO ("Mobility setup");
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobilityTracePath);
    ns2.Install (); // configure movements for each node, while reading trace file

    std::vector<std::vector<std::vector<bool>>> links;

    if(!linksPath.empty()) {
        links = loadLinks(linksPath);
        satHelper.getM_channel()->SetLinks(links);
    }

    //5. Install internet stack and routing:
    InternetStackHelper internet;
    LrrRoutingHelper lrrRouting;
    internet.SetRoutingHelper (lrrRouting);
    internet.Install (nodes);
    Ipv4AddressHelper ipAddrs;
    ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipAddrs.Assign (satHelper.getM_netDevices());

    //6. Install applications (ping)
    V4PingHelper ping (interfaces.GetAddress (2));
    ping.SetAttribute ("Verbose", BooleanValue (true));
    ApplicationContainer p = ping.Install (nodes.Get (0));
    p.Start (Seconds (0.001));
    p.Stop (Seconds (400));

    qemunet::Task task1 ("ping: qemu-1 -> qemu-3");
    task1.AddCommand ("qemu-1", "ping -c 30 10.0.0.3; route -n");
    std::string input ("/home/mike/ns-3-dev/src/satellites-blockchain/examples/small.ffs");
    Ptr<qemunet::TopologyHelper> qn = CreateObjectWithAttributes<qemunet::TopologyHelper> ("AutoStartServices", BooleanValue (true));
    qn->Create (input, task1.StartCallback ());

    //7. GO!
    lrr::GlobalGraph::Instance ()->Start ();
    Simulator::Stop (Seconds (500));
    Simulator::Run ();

    lrr::GlobalGraph::Instance ()->Stop ();

    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
	return 0;
}
