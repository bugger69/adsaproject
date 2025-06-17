// aqua-sim-routing-etr.cc
#include "aqua-sim-routing-etr.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "aqua-sim-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AquaSimEtrRouting");
NS_OBJECT_ENSURE_REGISTERED(AquaSimEtrRouting);

TypeId
AquaSimEtrRouting::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::AquaSimEtrRouting")
    .SetParent<AquaSimRouting>()
    .AddConstructor<AquaSimEtrRouting>();
  return tid;
}

AquaSimEtrRouting::AquaSimEtrRouting()
  : m_alpha(0.5), m_gamma(0.9), m_theta(0.2) {
  NS_LOG_FUNCTION(this);
}

bool
AquaSimEtrRouting::Recv(Ptr<Packet> packet, const Address &from) {
  NS_LOG_FUNCTION(this << packet);

  AquaSimHeader ash;
  packet->PeekHeader(ash);
  AquaSimAddress dst = ash.GetDAddr();

  if (ash.GetSAddr() == GetNetDevice()->GetAddress()) {
    for (auto neighbor : m_neighbors) {
      EvaluateTrust(neighbor);
    }

    AquaSimAddress nextHop = SelectNextHop();
    if (nextHop != AquaSimAddress::GetBroadcast()) {
      NS_LOG_INFO("ETR: Node " << GetNetDevice()->GetAddress() << " forwarding to " << nextHop);
      ash.SetNextHop(nextHop);
      packet->RemoveHeader(ash);
      packet->AddHeader(ash);
      SendDown(packet);
    }
    return true;
  }

  if (dst == GetNetDevice()->GetAddress()) {
    NS_LOG_INFO("ETR: Packet received at destination: " << dst);
    return true;
  }

  AquaSimAddress nextHop = SelectNextHop();
  if (nextHop != AquaSimAddress::GetBroadcast()) {
    NS_LOG_INFO("ETR: Node " << GetNetDevice()->GetAddress() << " relaying to " << nextHop);
    ash.SetNextHop(nextHop);
    packet->RemoveHeader(ash);
    packet->AddHeader(ash);
    SendDown(packet);
  }
  return true;
}

void
AquaSimEtrRouting::EvaluateTrust(const AquaSimAddress &node) {
  NS_LOG_FUNCTION(this << node);

  double reliability = 0.9; // placeholder
  double integrity = 0.95;
  double consistency = 0.85;
  double cooperation = 0.8;

  double trustScore = 0.3 * reliability + 0.25 * integrity + 0.25 * consistency + 0.2 * cooperation;
  m_trustTable[node] = trustScore;
  NS_LOG_INFO("ETR: Trust score for node " << node << " = " << trustScore);
}

AquaSimAddress
AquaSimEtrRouting::SelectNextHop() {
  NS_LOG_FUNCTION(this);
  AquaSimAddress best = AquaSimAddress::GetBroadcast();
  double max_q = -1.0;

  for (auto &entry : m_trustTable) {
    if (entry.second >= m_theta) {
      AquaSimAddress neighbor = entry.first;
      double q_val = m_qTable[{GetNetDevice()->GetAddress(), neighbor}];
      NS_LOG_INFO("ETR: Evaluating Q-value for " << neighbor << " = " << q_val);
      if (q_val > max_q) {
        max_q = q_val;
        best = neighbor;
      }
    }
  }

  if (best == AquaSimAddress::GetBroadcast()) {
    NS_LOG_WARN("ETR: No suitable next hop found.");
  } else {
    NS_LOG_INFO("ETR: Selected next hop: " << best);
  }

  return best;
}

} // namespace ns3
