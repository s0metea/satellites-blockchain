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
SatellitesHelper::ConfigureNodes (uint32_t satellites, uint32_t groundStations, DataRate dataRate, Time time)
{
  //Initial setup
  Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
  ObjectFactory m_propagationDelay;
  m_propagationDelay.SetTypeId ("ns3::ConstantSpeedPropagationDelayModel");
  Ptr<ConstantSpeedPropagationDelayModel> delay = m_propagationDelay.Create<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelay (delay);
  m_channel = channel;
  NodeContainer nodes;

  //Each ground station has its own gateway, so:
  uint32_t totalNodesAmount = satellites + 2 * groundStations;
  nodes.Create (totalNodesAmount);

  for(uint32_t i = 0; i < nodes.GetN(); i++) {
      Ptr<SatelliteNetDevice> device = CreateObject<SatelliteNetDevice>();
      Mac48Address mac = Mac48Address::Allocate();
      device->SetAddress(mac);
      device->SetDataRate(dataRate);
      device->SetInterframeGap(time);
      device->SetChannel(channel);
      channel->Add(device);
      nodes.Get(i)->AddDevice(device);
      m_netDevices.Add (device);
  }
  m_nodes = nodes;

  //First 40 nodes are satellites
  for(uint32_t i = 0; i < satellites; i ++) {
    m_satelliteDevices.Add(m_netDevices.Get(i));
  }
  for(uint32_t i = satellites; i < satellites + groundStations; i ++) {
    m_gatewayDevices.Add(m_netDevices.Get(i));
  }
  for(uint32_t i = satellites + groundStations; i < nodes.GetN(); i ++) {
    m_groundStationDevices.Add(m_netDevices.Get(i));
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
  for(uint32_t i = 0; i < m_groundStationDevices.GetN(); i ++) {
      //Setup tap devices
      std::string deviceNameBase ("tap");
      deviceNameBase.append (std::to_string (i));
      tapBridge.SetAttribute ("DeviceName", StringValue (deviceNameBase));
      tapBridge.Install (m_groundStationDevices.Get(i)->GetNode(), m_groundStationDevices.Get(i));
  }
  //We generate .conf files for nodes. LXC initialization should be done with .sh scripts.
  GenerateLXCConfigFiles();
  GenerateLXCRouteRules();
  return m_netDevices;
}

const
Ptr<SatelliteChannel> &SatellitesHelper::getM_channel () const
{
  return m_channel;
}

std::vector<SatelliteChannel::Links>
SatellitesHelper::LoadLinks (std::string filename){
  std::string time;
  std::vector<SatelliteChannel::Links> links;
  std::vector<std::vector<bool> > nodes;
  std::vector<bool> single_node;
  std::ifstream links_file;
  bool val;
  std::string row;
  uint32_t totalNodes = m_nodes.GetN();
  links_file.open (filename.c_str (), std::ios_base::in);
  while(std::getline (links_file, time))
    {
      for(uint32_t i = 0; i < totalNodes; i++)
        {
          std::getline (links_file, row);
          std::istringstream iss (row);
          for (uint32_t j = 0; j < totalNodes; j++)
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
    for(uint32_t i = 0; i < m_groundStationDevices.GetN(); i++) {
        //Creating LXC config and write it to .conf file
        std::string currentConfig;
        std::string lxcName("lxc");
        lxcName.append (std::to_string (i));
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
    for(uint32_t i = 0; i < m_groundStationDevices.GetN(); i++) {
        //Stop LXC
        std::string lxcName("lxc");
        lxcName.append (std::to_string (i));
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
    for(uint32_t i = 0; i < m_groundStationDevices.GetN(); i++) {
        //Starting LXC
        std::string lxcName("lxc");
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
    std::string lxcNetworkMask("/24");
    std::string lxcScriptUp("lxc.network.script.up = ");

    for(uint32_t i = 0; i < m_groundStationDevices.GetN(); i++) {
        //Creating LXC config and write it to .conf file
        std::string currentConfig;
        std::string lxcName("lxc");
        std::string tapName("tap");
        lxcName.append (std::to_string (i));
        tapName.append (std::to_string (i));
        currentConfig.append(lxcUtsName).append(lxcName).append("\n");
        currentConfig.append(lxcNetworkType).append("\n");
        currentConfig.append(lxcNetworkLink).append(tapName).append("\n");
        currentConfig.append(lxcNetworkName).append("\n");
        currentConfig.append(lxcNetworkIpv4);
        Ptr<Ipv4> ipv4 = m_groundStationDevices.Get(i)->GetNode()->GetObject<Ipv4> ();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
        std::string ip = ipToString(iaddr);
        currentConfig.append(ip);
        currentConfig.append(lxcNetworkMask).append("\n");
        std::string pathToLXCConf("./src/satellites-blockchain/lxc/conf/");
        std::ofstream out(pathToLXCConf.append(lxcName.append(".conf")));
        //currentConfig.append(lxcScriptUp).append("./conf/route/").append(lxcName).append(".sh\n");
        out << currentConfig;
        out.close();
    }
    return true;
}

bool
SatellitesHelper::GenerateLXCRouteRules() {
    std::string header("#!/bin/sh\n");
    std::string ipRouteAddDefault("ip route add default via ");
    std::string ipRouteAdd("ip route add ");
    for(uint32_t i = 0; i < m_groundStationDevices.GetN(); i++) {
        std::string pathToRouteRules("./src/satellites-blockchain/lxc/conf/route/");
        std::string script;
        script.append(header);
        std::string lxcName("lxc");
        lxcName.append(std::to_string(i));
        Ptr<Ipv4> ipv4 = m_gatewayDevices.Get(i)->GetNode()->GetObject<Ipv4> ();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
        std::string gwIP = ipToString(iaddr);
        script.append(ipRouteAddDefault).append(gwIP).append("\n");
        for(uint32_t j = 0; j < m_satelliteDevices.GetN(); j++) {
            Ptr<Ipv4> ipv4 = m_satelliteDevices.Get(j)->GetNode()->GetObject<Ipv4> ();
            Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
            std::string ip = ipToString(iaddr);
            script.append(ipRouteAdd).append(ip).append(" via ").append(gwIP).append("\n");
        }
        for(uint32_t j = 0; j < m_gatewayDevices.GetN(); j++) {
            Ptr<Ipv4> ipv4 = m_gatewayDevices.Get(j)->GetNode()->GetObject<Ipv4> ();
            Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
            std::string ip = ipToString(iaddr);
            script.append(ipRouteAdd).append(ip).append(" via ").append(gwIP).append("\n");
        }
        for(uint32_t j = 0; j < m_groundStationDevices.GetN(); j++) {
            if(i != j) {
                Ptr<Ipv4> ipv4 = m_groundStationDevices.Get(j)->GetNode()->GetObject<Ipv4>();
                Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
                std::string ip = ipToString(iaddr);
                script.append(ipRouteAdd).append(ip).append(" via ").append(gwIP).append("\n");
            }
        }
        std::ofstream out(pathToRouteRules.append(lxcName.append(".sh")));
        out << script;
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

std::string
SatellitesHelper::ipToString(Ipv4InterfaceAddress address) {
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

