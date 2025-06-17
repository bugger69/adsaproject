// aqua-sim-routing-gtr.cc
#include "aqua-sim-routing-gtr.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "aqua-sim-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AquaSimGtrRouting");
NS_OBJECT_ENSURE_REGISTERED(AquaSimGtrRouting);

TypeId
AquaSimGtrRouting::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::AquaSimGtrRouting")
    .SetParent<AquaSimRouting>()
    .AddConstructor<AquaSimGtrRouting>();
  return tid;
}

AquaSimGtrRouting::AquaSimGtrRouting()
  : m_alpha(0.5), m_gamma(0.9), m_phi(0.2) {
  NS_LOG_FUNCTION(this);
}

bool
AquaSimGtrRouting::Recv(Ptr<Packet> packet, const Address &from) {
  NS_LOG_FUNCTION(this << packet);

  AquaSimHeader ash;
  packet->PeekHeader(ash);
  AquaSimAddress dst = ash.GetDAddr();

  if (ash.GetSAddr() == GetNetDevice()->GetAddress()) {
    // Trust evaluation before forwarding
    for (auto neighbor : m_neighbors) {
      EvaluateTrust(neighbor);
    }

    AquaSimAddress nextHop = SelectNextHop();
    if (nextHop != AquaSimAddress::GetBroadcast()) {
      NS_LOG_INFO("Node " << GetNetDevice()->GetAddress() << " forwarding to " << nextHop);
      ash.SetNextHop(nextHop);
      packet->RemoveHeader(ash);
      packet->AddHeader(ash);
      SendDown(packet);
    }
    return true;
  }
  // Received packet from below
  if (dst == GetNetDevice()->GetAddress()) {
    NS_LOG_INFO("Packet received at final destination: " << dst);
    return true;
  }
  // Relay packet
  AquaSimAddress nextHop = SelectNextHop();
  if (nextHop != AquaSimAddress::GetBroadcast()) {
    NS_LOG_INFO("Node " << GetNetDevice()->GetAddress() << " relaying to " << nextHop);
    ash.SetNextHop(nextHop);
    packet->RemoveHeader(ash);
    packet->AddHeader(ash);
    SendDown(packet);
  }
  return true;
}

void
AquaSimGtrRouting::EvaluateTrust(const AquaSimAddress &node) {
  NS_LOG_FUNCTION(this << node);

  double trust_score = 0.5 + ((double)rand() / RAND_MAX - 0.5) * 0.4; // placeholder
  m_trustTable[node] = trust_score;
  NS_LOG_INFO("Node " << GetNetDevice()->GetAddress() << " evaluated trust of " << node << " = " << trust_score);
}

AquaSimAddress
AquaSimGtrRouting::SelectNextHop() {
  NS_LOG_FUNCTION(this);
  AquaSimAddress best = AquaSimAddress::GetBroadcast();
  double max_q = -1.0;

  for (auto &entry : m_trustTable) {
    if (std::abs(entry.second - 0.5) >= m_phi) {
      AquaSimAddress neighbor = entry.first;
      double q_val = m_qTable[{GetNetDevice()->GetAddress(), neighbor}];
      NS_LOG_INFO("Evaluating Q-value for " << neighbor << " = " << q_val);
      if (q_val > max_q) {
        max_q = q_val;
        best = neighbor;
      }
    }
  }

  if (best == AquaSimAddress::GetBroadcast()) {
    NS_LOG_WARN("No suitable next hop found.");
  } else {
    NS_LOG_INFO("Selected next hop: " << best);
  }

  return best;
}

} // namespace ns3
