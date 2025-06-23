#include "aqua-sim-routing-etr.h"
#include "aqua-sim-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AquaSimETRRouting");
NS_OBJECT_ENSURE_REGISTERED(AquaSimETRRouting);

TypeId
AquaSimETRRouting::GetTypeId() {
  static TypeId tid = TypeId("ns3::AquaSimETRRouting")
    .SetParent<AquaSimRouting>()
    .AddConstructor<AquaSimETRRouting>();
  return tid;
}

AquaSimETRRouting::AquaSimETRRouting()
{
  NS_LOG_FUNCTION(this);
  m_alpha = 0.1;
  m_gamma = 0.9;
  m_phi = 0.4;
}

void
AquaSimETRRouting::DoInitialize()
{
  NS_LOG_FUNCTION(this);
  AquaSimRouting::DoInitialize();  // Call base class init
  // No neighbor pre-population; will use dynamic discovery
}

bool
AquaSimETRRouting::Recv(Ptr<Packet> packet, const Address &from)
{
  NS_LOG_FUNCTION(this << packet << from);

  AquaSimAddress self = AquaSimAddress::ConvertFrom(GetNetDevice()->GetAddress());
  AquaSimAddress sender = AquaSimAddress::ConvertFrom(from);
  AquaSimHeader ash;
  packet->PeekHeader(ash);

  // ✅ Update neighbor discovery (based on sender)
  m_neighbors[sender] = Simulator::Now();
  if (m_trustTable.find(sender) == m_trustTable.end())
    m_trustTable[sender] = 1.0;
  if (m_qTable.find({self, sender}) == m_qTable.end())
    m_qTable[{self, sender}] = 0.5;

  if (ash.GetSAddr() == self) {
    NS_LOG_INFO("Packet originates from this node.");
  } else if (ash.GetDAddr() == self) {
    NS_LOG_INFO("Packet received at destination.");
    return true;
  } else {
    AquaSimAddress nextHop = SelectNextHop();
    if (nextHop == AquaSimAddress()) {
      NS_LOG_WARN("No next hop found. Dropping packet.");
      return false;
    }

    NS_LOG_INFO("Forwarding packet to next hop: " << nextHop);
    return SendDown(packet, nextHop, Seconds(0));
  }

  return true;
}

bool
AquaSimETRRouting::Recv(Ptr<Packet> packet, const Address &from, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION(this << packet << from << protocolNumber);
  return Recv(packet, from);
}

int64_t
AquaSimETRRouting::AssignStreams(int64_t stream)
{
  NS_LOG_FUNCTION(this << stream);
  return stream;
}

AquaSimAddress
AquaSimETRRouting::SelectNextHop()
{
  NS_LOG_FUNCTION(this);

  AquaSimAddress self = AquaSimAddress::ConvertFrom(GetNetDevice()->GetAddress());
  AquaSimAddress bestNeighbor;
  double bestScore = -std::numeric_limits<double>::infinity();

  Time now = Simulator::Now();
  Time neighborTimeout = Seconds(30);  // Expiry threshold

  for (auto it = m_neighbors.begin(); it != m_neighbors.end(); ) {
    AquaSimAddress neighbor = it->first;
    Time lastHeard = it->second;

    if ((now - lastHeard) > neighborTimeout) {
      NS_LOG_WARN("Neighbor " << neighbor << " expired, last seen at " << lastHeard.GetSeconds());
      it = m_neighbors.erase(it);
      continue;
    }

    std::pair<AquaSimAddress, AquaSimAddress> key(self, neighbor);
    double q_val = m_qTable[key];

    NS_LOG_INFO("Neighbor: " << neighbor << ", Q-value: " << q_val);
    if (q_val > bestScore) {
      bestScore = q_val;
      bestNeighbor = neighbor;
    }

    ++it;
  }

  if (bestNeighbor == AquaSimAddress()) {
    NS_LOG_WARN("ETR SelectNextHop: No suitable neighbor selected.");
  } else {
    NS_LOG_INFO("ETR SelectNextHop: Chosen next hop = " << bestNeighbor);
  }

  return bestNeighbor;
}

void
AquaSimETRRouting::EvaluateTrust(const AquaSimAddress &node)
{
  NS_LOG_FUNCTION(this << node);
  if (m_trustTable[node] < m_phi) {
    NS_LOG_WARN("Trust too low for node " << node << ": " << m_trustTable[node]);
  } else {
    NS_LOG_INFO("Trust OK for node " << node << ": " << m_trustTable[node]);
  }
}

void
AquaSimETRRouting::UpdateQTable(const AquaSimAddress &s, const AquaSimAddress &a, double reward)
{
  NS_LOG_FUNCTION(this << s << a << reward);
  std::pair<AquaSimAddress, AquaSimAddress> key(s, a);
  double oldVal = m_qTable[key];
  m_qTable[key] = oldVal + m_alpha * (reward + m_gamma * GetMaxQ(a) - oldVal);
  NS_LOG_INFO("Updated Q-table for " << s << " -> " << a << ": " << m_qTable[key]);
}

double
AquaSimETRRouting::GetMaxQ(const AquaSimAddress &state)
{
  NS_LOG_FUNCTION(this << state);

  double maxQ = -1.0;
  for (const auto &entry : m_neighbors) {
    AquaSimAddress a = entry.first;
    std::pair<AquaSimAddress, AquaSimAddress> key(state, a);
    double q = m_qTable[key];
    if (q > maxQ) maxQ = q;
  }

  return (maxQ > 0) ? maxQ : 0.0;
}

void
AquaSimETRRouting::LoadTrustScores(const std::string &filename)
{
  NS_LOG_INFO("ETR loading trust scores from: " << filename);
  // Placeholder: add file-based initialization if needed
}

} // namespace ns3
