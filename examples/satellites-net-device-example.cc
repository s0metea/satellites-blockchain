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
#include "ns3/csma-module.h"
#include "ns3/tap-bridge-module.h"
#include <ns3/v4ping-helper.h>
#include <ns3/satellites-blockchain-helper.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("SatellitesExample");

map<double, vector<vector<bool>>> loadLinks(string filename){
    int total_nodes = 3;
    double time = 0.0;
    map<double, vector<vector<bool>>> links;
    vector<vector<bool>> nodes;
    vector<bool> single_node;
    ifstream links_file;
    bool val;
    links_file.open(filename.c_str(), ios_base::in);
    while(links_file >> val) {
        links_file >> time;
        for(int i = 0; i < total_nodes; i++) {
            for (int j = 0; j < total_nodes; j++) {
                links_file >> val;
                single_node.push_back(val);
            }
            nodes.push_back(single_node);
            single_node.clear();
        }
        links.insert(std::pair<double, vector<vector<bool>>>(time, nodes));
        nodes.clear();
    }
    links_file.close();
    return links;
}

int
main(int argc, char *argv[]) {
    LogComponentEnable("TapBridge", LOG_LEVEL_INFO);
    //LogComponentEnable ("ns3::SatelliteChannel", LOG_LEVEL_DEBUG);
    //LogComponentEnable ("ns3::SatelliteNetDevice", LOG_LEVEL_DEBUG);

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

    //
    // We are interacting with the outside, real, world.  This means we have to
    // interact in real-time and therefore means we have to use the real-time
    // simulator and take the time to calculate checksums.
    //
    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    NS_LOG_INFO ("Objects creation");
	NodeContainer nodes;
	SatellitesHelper satHelper;
    nodes = satHelper.ConfigureNodes(2, DataRate ("100MB/s"), Time(0));

    NS_LOG_INFO ("Mobility setup");
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobilityTracePath);
    ns2.Install (); // configure movements for each node, while reading trace file

    std::map<double, std::vector<std::vector<bool>>> links;
    if(!linksPath.empty()) {
        links = loadLinks(linksPath);
        satHelper.getM_channel()->SetLinks(links);
    }

    
    //LrrRoutingHelper lrrRouting;
    //internet.SetRoutingHelper (lrrRouting);

    
    //
    // Connect the right side tap to the right side device on the right-side
    // ghost node.
    //

    //5. Install internet stack and routing:
    InternetStackHelper internet;
    internet.Install (nodes);


    NetDeviceContainer devices;
    for(unsigned int i =0; i < 2; i++)
    {
        Ptr<NetDevice> d = nodes.Get(i)->GetDevice(0);
        devices.Add(d);
    }

    Ipv4AddressHelper addresses;
    addresses.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = addresses.Assign (devices);


//
    // 6. Use the TapBridgeHelper to connect to the pre-configured tap devices for
    // the left side.  We go with "UseBridge" mode.
    //
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute ("Mode", StringValue ("ConfigureLocal"));
    tapBridge.SetAttribute ("DeviceName", StringValue ("tap-1"));
    tapBridge.Install (nodes.Get (0), satHelper.getM_netDevices().Get (0));
    tapBridge.SetAttribute ("DeviceName", StringValue ("tap-2"));
    tapBridge.Install (nodes.Get (1), satHelper.getM_netDevices().Get (0));



    //7. GO!
    //lrr::GlobalGraph::Instance ()->Start ();
    Simulator::Stop (Seconds (650));
    Simulator::Run ();

    //lrr::GlobalGraph::Instance ()->Stop ();

    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
	return 0;
}

