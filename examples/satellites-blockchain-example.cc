#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>
#include <ns3/mobility-module.h>
#include <ns3/low-resolution-radio-module.h>
#include <ns3/trace-helper.h>
#include "ns3/tap-bridge-module.h"
#include <ns3/satellites-helper.h>
#include <ns3/lrr-routing-topology.h>

using namespace ns3;
using namespace std;

int main (int argc, char *argv[]) {
  LogComponentEnable ("TapBridge", LOG_LEVEL_ALL);
  LogComponentEnable ("ns3::SatelliteChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("ns3::SatelliteNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("LrrRoutingProtocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);


  // Specify default paths for this toy example
  string mobilityTracePath = "./src/satellites-blockchain/examples/mobility.trace";
  string linksPath = "./src/satellites-blockchain/examples/links";

  CommandLine cmd;
  cmd.AddValue ("tracePath", "Path to the NS2Mobility trace file", mobilityTracePath);
  cmd.AddValue ("linksPath", "Path to the links", linksPath);
  cmd.Parse (argc,argv);

  //
  // We are interacting with the outside, real, world.  This means we have to
  // interact in real-time and therefore means we have to use the real-time
  // simulator and take the time to calculate checksums.
  //
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  NetDeviceContainer devices;
  SatellitesHelper satHelper;
  uint32_t nNodes = 3;
  // TODO: return net device container here
  devices = satHelper.ConfigureNodes (nNodes, DataRate ("100MB/s"), Time (0));
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobilityTracePath);
  ns2.Install ();   // configure movements for each node, while reading trace file

  std::vector<SatelliteChannel::Links> links;
  if(!linksPath.empty ())
  {
      links = satHelper.LoadLinks (linksPath, nNodes);
      satHelper.getM_channel ()->SetLinks (links);
  }

  lrr::GlobalGraph::Instance ()->Start ();
  //Simulator::Stop(Time("300s"));
  Simulator::Run ();
  //std::ofstream s("topology.txt", std::ofstream::out);
  //lrr::GlobalGraph::Instance()->PrintGraph(s);
  //s.close();
  lrr::GlobalGraph::Instance ()->Stop ();

  Simulator::Destroy ();
  return 0;
}

