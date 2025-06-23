#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h"
#include "ns3/aqua-sim-ng-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/callback.h"

#include <set>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ETRTestDiagnostic");

int main(int argc, char *argv[]) {
  double simStop = 200;
  int nodes = 200;
  int sinks = 1;
  int numMalicious = 56;
  uint32_t m_dataRate = 10000;
  uint32_t m_packetSize = 40;
  double range = 100;  // Increased for connectivity diagnostics

  LogComponentEnable("ETRTestDiagnostic", LOG_LEVEL_INFO);
  LogComponentEnable("AquaSimETRRouting", LOG_LEVEL_INFO);

  CommandLine cmd;
  cmd.AddValue("simStop", "Length of simulation", simStop);
  cmd.AddValue("nodes", "Amount of regular underwater nodes", nodes);
  cmd.Parse(argc, argv);

  std::cout << "-----------Initializing ETR Diagnostic Simulation-----------\n";

  NodeContainer nodesCon, sinksCon, senderCon;
  nodesCon.Create(nodes);
  sinksCon.Create(sinks);
  senderCon.Create(1);

  PacketSocketHelper socketHelper;
  socketHelper.Install(nodesCon);
  socketHelper.Install(sinksCon);
  socketHelper.Install(senderCon);

  AquaSimChannelHelper channel = AquaSimChannelHelper::Default();
  channel.SetPropagation("ns3::AquaSimRangePropagation");

  AquaSimHelper asHelper = AquaSimHelper::Default();
  asHelper.SetChannel(channel.Create());
  asHelper.SetMac("ns3::AquaSimBroadcastMac");

  MobilityHelper mobility, nodeMobility;
  Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
  NetDeviceContainer devices;

  // Random malicious node selection
  std::set<uint32_t> maliciousNodeIds;
  Ptr<UniformRandomVariable> randGen = CreateObject<UniformRandomVariable>();
  while (maliciousNodeIds.size() < (uint32_t)numMalicious) {
    maliciousNodeIds.insert(randGen->GetInteger(0, nodes - 1));
  }

  for (uint32_t i = 0; i < (uint32_t)nodes; ++i) {
    Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice>();
    asHelper.SetRouting("ns3::AquaSimETRRouting");  // set same routing for all for now

    devices.Add(asHelper.Create(nodesCon.Get(i), newDevice));
    newDevice->GetPhy()->SetTransRange(range);

    if (maliciousNodeIds.count(i)) {
      NS_LOG_WARN("⚠️ ETR Diagnostic: Node " << i << " is MALICIOUS → dropping all packets.");
      Ptr<NetDevice> netDev = newDevice;
      netDev->SetReceiveCallback(MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address &>());
    }
  }

  // Sink node
  for (NodeContainer::Iterator i = sinksCon.Begin(); i != sinksCon.End(); ++i) {
    Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice>();
    position->Add(Vector(190, 190, 0));
    devices.Add(asHelper.Create(*i, newDevice));
    newDevice->GetPhy()->SetTransRange(range);
  }

  // Sender node
  Ptr<AquaSimNetDevice> sendDevice = CreateObject<AquaSimNetDevice>();
  position->Add(Vector(10, 10, 0));
  devices.Add(asHelper.Create(senderCon.Get(0), sendDevice));
  sendDevice->GetPhy()->SetTransRange(range);

  mobility.SetPositionAllocator(position);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  nodeMobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue(100.0),
                                    "Y", DoubleValue(100.0),
                                    "rho", DoubleValue(100));
  nodeMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  nodeMobility.Install(nodesCon);
  mobility.Install(sinksCon);
  mobility.Install(senderCon);

  PacketSocketAddress socket;
  socket.SetAllDevices();

  // Sender device is at index nodes + sinks
  socket.SetPhysicalAddress(devices.Get(nodes + sinks)->GetAddress());
  socket.SetProtocol(0);

  OnOffHelper app("ns3::PacketSocketFactory", Address(socket));
  app.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0066]"));
  app.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.9934]"));
  app.SetAttribute("DataRate", DataRateValue(m_dataRate));
  app.SetAttribute("PacketSize", UintegerValue(m_packetSize));

  ApplicationContainer apps = app.Install(senderCon);
  apps.Start(Seconds(0.5));
  apps.Stop(Seconds(simStop));

  Ptr<Node> sinkNode = sinksCon.Get(0);
  TypeId psfid = TypeId::LookupByName("ns3::PacketSocketFactory");
  Ptr<Socket> sinkSocket = Socket::CreateSocket(sinkNode, psfid);
  sinkSocket->Bind(socket);

  Packet::EnablePrinting();
  std::cout << "-----------Running ETR Diagnostic Simulation-----------\n";

  Simulator::Stop(Seconds(simStop));
  Simulator::Run();

  asHelper.GetChannel()->PrintCounters();  // Packet stats summary
  Simulator::Destroy();

  std::cout << "fin.\n";
  return 0;
}
