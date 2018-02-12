#include <vector>
#include <string>
#include <ns3/node-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>
#include <ns3/core-module.h>
#include <ns3/tap-bridge-module.h>
#include <ns3/internet-module.h>
#include <ns3/lrr-routing-helper.h>
#include "satellites-helper.h"

namespace ns3 {
NetDeviceContainer
SatellitesHelper::ConfigureNodes (uint32_t nodes_amount, DataRate dataRate, Time time)
{
  //Initial setup
  Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
  ObjectFactory m_propagationDelay;
  m_propagationDelay.SetTypeId ("ns3::ConstantSpeedPropagationDelayModel");
  Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
  channel->SetPropagationDelay (delay);
  m_channel = channel;
  NodeContainer nodes;
  nodes.Create (nodes_amount * 2);

  for(uint32_t i = 0; i < nodes_amount * 2; i++) {
      Ptr<SatelliteNetDevice> device = CreateObject<SatelliteNetDevice>();
      std::string macBase("00:00:00:00:01:0");
      macBase.append(std::to_string(i + 1));
      Mac48Address mac = Mac48Address(macBase.c_str());
      std::cout << mac << std::endl;
      device->SetAddress(mac);
      device->SetDataRate(dataRate);
      device->SetInterframeGap(time);
      device->SetChannel(channel);
      channel->Add(device);
      nodes.Get(i)->AddDevice(device);
      m_netDevices.Add (device);
  }
  m_nodes = nodes;

  NetDeviceContainer gatewayDevices, satellitesDevices;
  for(uint32_t i = 0; i < (nodes_amount * 2) - 1; i += 2) {
    satellitesDevices.Add(m_netDevices.Get(i));
    gatewayDevices.Add(m_netDevices.Get(i + 1));
  }

  //Install internet stack and routing:
  InternetStackHelper internet;
  //Routing by LRR:
  LrrRoutingHelper lrrRouting;
  internet.SetRoutingHelper (lrrRouting);
  internet.Install (nodes);
  Ipv4AddressHelper addresses;
  addresses.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = addresses.Assign (m_netDevices);

  // Use the TapBridgeHelper to connect to the pre-configured tap devices.
  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("ConfigureLocal"));
  uint32_t tapIndex = 1;
  uint32_t deviceIndex = 0;
  for(uint32_t i = 0; i < nodes_amount * 2; i += 2) {
      //Setup tap devices
      std::string deviceNameBase ("tap-");
      deviceNameBase.append (std::to_string (tapIndex));
      tapBridge.SetAttribute ("DeviceName", StringValue (deviceNameBase));
      tapBridge.Install (nodes.Get (i), satellitesDevices.Get(deviceIndex));
      tapIndex++;
      deviceIndex++;
  }
  //We generate .conf files for nodes. LXC initialization should be done with .sh scripts.
  GenerateLXCConfigFiles();

  return m_netDevices;
}

const
Ptr<SatelliteChannel> &SatellitesHelper::getM_channel () const
{
  return m_channel;
}

std::vector<SatelliteChannel::Links>
SatellitesHelper::LoadLinks (std::string filename, int totalNodes){
  std::string time;
  std::vector<SatelliteChannel::Links> links;
  std::vector<std::vector<bool> > nodes;
  std::vector<bool> single_node;
  std::ifstream links_file;
  bool val;
  std::string row;
  links_file.open (filename.c_str (), std::ios_base::in);
  while(std::getline (links_file, time))
    {
      for(int i = 0; i < totalNodes * 2; i++)
        {
          std::getline (links_file, row);
          std::istringstream iss (row);
          for (int j = 0; j < totalNodes * 2; j++)
            {
              iss >> val;
              single_node.push_back (val);
            }
          nodes.push_back (single_node);
          single_node.clear ();
        }
      links.push_back (SatelliteChannel::Links (Time (time), nodes));
      nodes.clear ();
    }
  links_file.close ();
  return links;
}

const NodeContainer
&SatellitesHelper::getM_nodes() const {
    return m_nodes;
}

bool
SatellitesHelper::CreateLXC() {
    for(uint32_t i = 0; i < getM_nodes().GetN() / 2; i++) {
        //Creating LXC config and write it to .conf file
        std::string currentConfig;
        std::string lxcName("lxc-");
        lxcName.append (std::to_string (i + 1));
        //Creating LXC:
        std::string createCommand("lxc-create -f ./");
        createCommand.append(lxcName);
        createCommand.append(".conf -n ");
        createCommand.append(lxcName);
        createCommand.append(" -t ubuntu");
        system(createCommand.c_str());
    }
    system("lxc-ls -f");
    return true;
}

bool
SatellitesHelper::DestroyLXC() {
    for(uint32_t i = 0; i < getM_nodes().GetN() / 2; i++) {
        //Stop LXC
        std::string lxcName("lxc-");
        lxcName.append (std::to_string (i + 1));
        std::string stopCommand("lxc-stop -n ");
        stopCommand.append(lxcName);
        stopCommand.append(" -k");
        system(stopCommand.c_str());
        //DestroyLXC LXC
        std::string destroyCommand("lxc-destroy -n ");
        destroyCommand.append(lxcName);
        system(destroyCommand.c_str());
    }
    system("lxc-ls -f");
    return true;
}

bool
SatellitesHelper::RunLXC() {
    for(uint32_t i = 0; i < getM_nodes().GetN() / 2; i++) {
        //Starting LXC
        std::string lxcName("lxc-");
        std::string startCommand("lxc-start -n ");
        startCommand.append(lxcName);
        system(startCommand.c_str());
    }
    system("lxc-ls -f");
    return true;
}

bool
SatellitesHelper::GenerateLXCConfigFiles() {
    std::string lxcUtsName("lxc.utsname = ");
    std::string lxcNetworkType("lxc.network.type = phys");
    std::string lxcNetworkLink("lxc.network.link = ");
    std::string lxcNetworkName("lxc.network.name = eth0");
    std::string lxcNetworkIpv4("lxc.network.ipv4 = ");
    std::string lxcNetworkGateway("lxc.network.ipv4.gateway = ");
    std::string lxcNetworkMask("/24");
    for(uint32_t i = 0; i < getM_nodes().GetN() / 2; i++) {
        //Creating LXC config and write it to .conf file
        std::string currentConfig;
        std::string lxcName("lxc-");
        std::string tapName("tap-");
        lxcName.append (std::to_string (i + 1));
        tapName.append (std::to_string (i + 1));
        currentConfig.append(lxcUtsName).append(lxcName).append("\n");
        currentConfig.append(lxcNetworkType).append("\n");
        currentConfig.append(lxcNetworkLink).append(tapName).append("\n");
        currentConfig.append(lxcNetworkName).append("\n");
        currentConfig.append(lxcNetworkIpv4);
        Ptr<Ipv4> ipv4 = m_nodes.Get(i * 2)->GetObject<Ipv4> ();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
        std::string ip = ipToString(iaddr);
        currentConfig.append(ip);
        currentConfig.append(lxcNetworkMask).append("\n");
        //Gateway:
        ipv4 = m_nodes.Get(i * 2 + 1)->GetObject<Ipv4> ();
        iaddr = ipv4->GetAddress(1, 0);
        std::string gatewayIP = ipToString(iaddr);
        currentConfig.append(lxcNetworkGateway).append(gatewayIP).append("\n");
        std::ofstream out(lxcName.append(".conf"));
        out << currentConfig;
        out.close();
    }
    return true;
}

bool
SatellitesHelper::SetupLXC() {
    std::cout << "Previous LXC destroy..." << std::endl;
    DestroyLXC();
    std::cout << "Done!" << std::endl;
    std::cout << "LXC config file generation..." << std::endl;
    if (GenerateLXCConfigFiles()) {
        std::cout << "Done!" << std::endl;
        std::cout << "LXC creation..." << std::endl;
        if (CreateLXC()) {
            std::cout << "Done!" << std::endl;
            std::cout << "LXC running..." << std::endl;
            RunLXC();
            std::cout << "Done!" << std::endl;
        }
    }
    return true;
}

    std::string SatellitesHelper::ipToString(Ipv4InterfaceAddress address) {
        char buf[4];
        address.GetLocal().Serialize((uint8_t *) buf);
        std::string ip;
        ip.append(
                std::to_string(buf[0])).append(".").
                append(std::to_string(buf[1])).append(".").
                append(std::to_string(buf[2])).append(".").
                append(std::to_string(buf[3]));
        return ip;
    }
}

