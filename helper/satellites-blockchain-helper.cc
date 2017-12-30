/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/node-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-net-device.h>
#include <ns3/core-module.h>
#include "satellites-blockchain-helper.h"

namespace ns3 {
    NodeContainer SatellitesHelper::ConfigureNodes(uint32_t nodes_amount, DataRate dataRate, Time time) {
        Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel> ();
        ObjectFactory m_propagationDelay;
        m_propagationDelay.SetTypeId("ns3::ConstantSpeedPropagationDelayModel");
        Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
        channel->SetPropagationDelay (delay);
        m_channel = channel;
        NodeContainer nodes;
        nodes.Create(nodes_amount);
        for(uint32_t i = 0; i < nodes_amount; i++) {
            Ptr<SatelliteNetDevice> device = CreateObject<SatelliteNetDevice> ();
            device->SetAddress(Mac48Address::Allocate ());
            device->SetDataRate(dataRate);
            device->SetInterframeGap(time);
            channel->Add(device);
            device->SetChannel(channel);
            nodes.Get(i)->AddDevice(device);
            m_netDevices.Add(device);
        }
        return nodes;
    }

    const Ptr<SatelliteChannel> &SatellitesHelper::getM_channel() const {
        return m_channel;
    }

    const NetDeviceContainer &SatellitesHelper::getM_netDevices() const {
        return m_netDevices;
    }

    void SatellitesHelper::setM_netDevices(const NetDeviceContainer &m_netDevices) {
        SatellitesHelper::m_netDevices = m_netDevices;
    }
}

