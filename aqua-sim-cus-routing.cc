// Filename: aqua-sim-gtr-routing.cc

#include "aqua-sim-gtr-routing.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AquaSimGTRRouting");

NS_OBJECT_ENSURE_REGISTERED(AquaSimGTRRouting);

TypeId
AquaSimGTRRouting::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::AquaSimGTRRouting")
    .SetParent<AquaSimRouting>()
    .AddConstructor<AquaSimGTRRouting>();
  return tid;
}

AquaSimGTRRouting::AquaSimGTRRouting()
{
  m_alpha = 0.1; // Learning rate
  m_gamma = 0.9; // Discount factor
  m_rand = CreateObject<UniformRandomVariable>();
}

void
AquaSimGTRRouting::DoDispose()
{
  AquaSimRouting::DoDispose();
}

bool
AquaSimGTRRouting::Recv(Ptr<Packet> packet, const Address &address)
{
  AquaSimHeader ash;
  packet->PeekHeader(ash);

  if (ash.GetNextHop() == GetNetDevice()->GetAddress() || ash.GetNextHop().IsInvalid())
  {
    if (ash.GetSAddr() == GetNetDevice()->GetAddress())
    {
      // Source node: set next hop
      Address nextHop = SelectBestNextHop();
      ash.SetNextHop(nextHop);
      packet->AddHeader(ash);
      SendDown(packet, nextHop);
      return true;
    }
    else if (ash.GetDAddr() == GetNetDevice()->GetAddress())
    {
      // Destination node
      NS_LOG_INFO("Packet received at destination");
      return true;
    }
    else
    {
      // Intermediate node: forward
      Address nextHop = SelectBestNextHop();
      ash.SetNextHop(nextHop);
      packet->AddHeader(ash);
      SendDown(packet, nextHop);
      return true;
    }
  }

  return false;
}

Address
AquaSimGTRRouting::SelectBestNextHop()
{
  Address best;
  double bestValue = -1e9;

  for (auto &entry : m_qTable)
  {
    double trust = CalculateTrust(entry.first);
    double qValue = entry.second;
    double score = trust + qValue;

    if (score > bestValue)
    {
      best = entry.first;
      bestValue = score;
    }
  }

  return best;
}

double
AquaSimGTRRouting::CalculateTrust(Address neighbor)
{
  if (m_trustScores.find(neighbor) != m_trustScores.end())
  {
    return m_trustScores[neighbor];
  }
  return 0.5; // Default trust
}

void
AquaSimGTRRouting::UpdateQValue(Address current, Address nextHop, double reward)
{
  double oldQ = m_qTable[nextHop];
  double maxFutureQ = 0; // No future knowledge for now

  m_qTable[nextHop] = oldQ + m_alpha * (reward + m_gamma * maxFutureQ - oldQ);
}
