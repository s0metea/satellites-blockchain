#include <iostream>
#include <fstream>
#include <sstream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "json.hpp"
#include "ns3/sat-channel.h"
#include "ns3/sat-net-device.h"
#include "ns3/mac48-address.h"
#include "ns3/satellite-module.h"

using namespace ns3;
using namespace std;
using json = nlohmann::json;

std::string trace_file = "./data/data.trace";
std::string log_file = "./data/log.txt";
std::string links_file = "./data/links";

uint32_t satellites_amount = 0;
uint32_t ground_stations_amount = 0;
unsigned int    nodeNum = 0;
double duration;


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

static Ptr<SatNetDevice> CreateSimpleDevice (Ptr<Node> node)
{
    //Ptr<OpticalNetDevice> device = CreateObject<OpticalNetDevice> ();
    //device->SetAddress(Mac48Address::Allocate());
    //node->AddDevice(device);
    return NULL;
}


static void CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility) {
    Vector pos = mobility->GetPosition (); // Get position
    Vector vel = mobility->GetVelocity (); // Get velocity
    // Prints position and velocities
    *os << Simulator::Now () << " POS: x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z  << " VEL: x=" << vel.x << ", y=" << vel.y << ", z=" << vel.z << endl;
    *os << foo << endl;
    //Here we need to modify available channel for the node net device
    Ptr<SatChannel> channel = CreateObject<SatChannel> ();
    //device->SetChannel(channel);
}


int main (int argc, char *argv[]) {
    std::ifstream objects("./data/objects.json");
    json j;
    objects >> j;
    satellites_amount = j["satellites"].size();
    ground_stations_amount = j["groundstations"].size();
    //satellites_amount = 40;
    //ground_stations_amount = 9;
    cout << "Satellites: " << satellites_amount << endl;
    cout << "Ground stations: " << ground_stations_amount << endl;

    nodeNum = satellites_amount + ground_stations_amount;

    // Enable logging from the ns2 helper
    LogComponentEnable ("Ns2MobilityHelper", LOG_LEVEL_DEBUG);

    // open log file for output
    std::ofstream os;
    os.open (log_file.c_str ());

    // Nodes setup:
    NodeContainer satellites, ground_stations, nodes;
    satellites.Create (satellites_amount);
    ground_stations.Create (ground_stations_amount);
    nodes.Add(satellites);
    nodes.Add(ground_stations);
    // Create Ns2MobilityHelper with the specified trace log file as parameter
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (trace_file);
    ns2.Install (); // configure movements for each node, while reading trace file

    //NetDevice setup:
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        CreateSimpleDevice(nodes.Get(i));
    }

    //Channels setup:
    //cout << "Reading links file..." << endl;
    //vector<vector<vector<bool>>> links = loadLinks(links_file);
    //cout << "Done!" << endl;

    int moment_of_time = 0;

    // Configure callback for logging
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeBoundCallback (&CourseChange, &os));
    Simulator::Run ();
    Simulator::Destroy ();
    os.close (); // close log file
    return 0;
}
