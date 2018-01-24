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

int
main(int argc, char *argv[]) {
    LogComponentEnable("TapBridge", LOG_LEVEL_INFO);
    LogComponentEnable ("ns3::SatelliteChannel", LOG_LEVEL_ALL);
    //LogComponentEnable ("ns3::SatelliteNetDevice", LOG_LEVEL_ALL);
    LogComponentEnable("SatellitesExample", LOG_LEVEL_INFO);

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
    uint32_t totalNodesAmount = 2;
    nodes = satHelper.ConfigureNodes(totalNodesAmount, DataRate ("100MB/s"), Time(0));

    NS_LOG_INFO ("Mobility setup");
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobilityTracePath);
    ns2.Install (); // configure movements for each node, while reading trace file

    std::vector<SatelliteChannel::Links> links;
    if(!linksPath.empty()) {
        links = satHelper.LoadLinks(linksPath, totalNodesAmount);
        satHelper.getM_channel()->SetLinks(links);
        NS_LOG_INFO ("Links was loaded!");
    } else {
        NS_LOG_INFO ("Links wasn't loaded!");
    }

    //5. Install internet stack and routing:
    InternetStackHelper internet;
    internet.Install (nodes);
    Ipv4AddressHelper addresses;
    addresses.SetBase ("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = addresses.Assign (satHelper.getM_netDevices());

    LrrRoutingHelper lrrRouting;
    internet.SetRoutingHelper (lrrRouting);

    // 6. Use the TapBridgeHelper to connect to the pre-configured tap devices.
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute ("Mode", StringValue ("UseBridge"));
    for(uint32_t i = 0; i < totalNodesAmount; i++) {
        string deviceNameBase("tap-");
        deviceNameBase.append(to_string(i + 1));
        tapBridge.SetAttribute ("DeviceName", StringValue(deviceNameBase));
        tapBridge.Install (nodes.Get (i), satHelper.getM_netDevices().Get(i));
    }

    //7. GO!
    lrr::GlobalGraph::Instance ()->Start ();
    Simulator::Stop (Seconds (650));
    Simulator::Run ();

    lrr::GlobalGraph::Instance ()->Stop ();

    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
	return 0;
}

