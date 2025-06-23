#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aqua-sim-ng-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <map>
#include <set>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ETRMetricsTest");

// Data structures for metrics
struct NodeStats {
  uint32_t sent = 0;
  uint32_t received = 0;
  uint32_t authentic = 0;
  uint32_t transmitted = 0;
  uint32_t expectedTransmissions = 0;
  uint32_t relayed = 0;
  uint32_t expectedRelayed = 0;
};

std::map<uint32_t, NodeStats> stats;
std::set<uint32_t> maliciousNodes;

void TxCallback(Ptr<const Packet> packet, const Address& from) {
  uint32_t id = Simulator::GetContext();
  stats[id].sent++;
  stats[id].transmitted++;
}

void RxCallback(Ptr<const Packet> packet, const Address& from) {
  uint32_t id = Simulator::GetContext();
  stats[id].received++;
}

void RelayCallback(Ptr<const Packet> packet) {
  uint32_t id = Simulator::GetContext();
  stats[id].relayed++;
}

void PrintResults() {
  std::ofstream out("metrics_output.csv");
  out << "Node,Reliability,Integrity,BehavioralConsistency,Cooperation\n";

  for (auto& [id, s] : stats) {
    double reliability = (s.sent > 0) ? double(s.received) / s.sent : 0;
    double integrity = (s.sent > 0) ? double(s.authentic) / s.sent : 0;
    double maxDev = 100;  // tune as needed
    double behavior = 1.0 - std::min(1.0, std::abs(int(s.transmitted - s.expectedTransmissions)) / maxDev);
    double cooperation = (s.expectedRelayed > 0) ? double(s.relayed) / s.expectedRelayed : 0;

    out << id << "," << reliability << "," << integrity << "," << behavior << "," << cooperation << "\n";
  }
  out.close();
  NS_LOG_UNCOND("Metrics written to metrics_output.csv");
}

int main(int argc, char *argv[]) {
  uint32_t totalNodes = 100, maliciousCount = 10;

  NodeContainer nodes;
  nodes.Create(totalNodes);

  AquaSimChannelHelper channel = AquaSimChannelHelper::Default();
  channel.SetPropagation("ns3::AquaSimRangePropagation");

  AquaSimHelper asHelper;
  asHelper.SetChannel(channel.Create());
  asHelper.SetMac("ns3::AquaSimBroadcastMac");
  asHelper.SetRouting("ns3::AquaSimETRRouting");

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  NetDeviceContainer devices;
  for (uint32_t i = 0; i < totalNodes; ++i) {
    Ptr<AquaSimNetDevice> dev = CreateObject<AquaSimNetDevice>();
    Ptr<Node> node = nodes.Get(i);
    devices.Add(asHelper.Create(node, dev));

    dev->TraceConnectWithoutContext("Tx", MakeCallback(&TxCallback));
    dev->TraceConnectWithoutContext("Rx", MakeCallback(&RxCallback));
    // You can also attach additional custom trace hooks for packet drop or auth.
  }

  Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
  while (maliciousNodes.size() < maliciousCount) {
    maliciousNodes.insert(rng->GetInteger(0, totalNodes - 1));
  }

  for (uint32_t i = 0; i < totalNodes; ++i) {
    if (maliciousNodes.count(i)) {
      NS_LOG_WARN("Node " << i << " is malicious.");
      Ptr<NetDevice> dev = nodes.Get(i)->GetDevice(0);
      dev->SetReceiveCallback(MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address &>());
    }
  }

  // Simple OnOff App from node 0 to node N-1
  PacketSocketAddress socket;
  socket.SetAllDevices();
  socket.SetPhysicalAddress(devices.Get(totalNodes - 1)->GetAddress());
  socket.SetProtocol(0);

  OnOffHelper onoff("ns3::PacketSocketFactory", Address(socket));
  onoff.SetAttribute("DataRate", DataRateValue(DataRate("5kbps")));
  onoff.SetAttribute("PacketSize", UintegerValue(40));
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer app = onoff.Install(nodes.Get(0));
  app.Start(Seconds(1));
  app.Stop(Seconds(200));

  Simulator::Stop(Seconds(200));
  Simulator::Run();

  PrintResults();

  Simulator::Destroy();
  return 0;
}

