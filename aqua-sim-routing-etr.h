#ifndef AQUA_SIM_ROUTING_ETR_H
#define AQUA_SIM_ROUTING_ETR_H

#include "aqua-sim-routing.h"
#include "ns3/random-variable-stream.h"
#include "ns3/node-list.h"
#include <map>
#include <vector>

namespace ns3 {

class AquaSimETRRouting : public AquaSimRouting {
public:
  static TypeId GetTypeId(void);
  AquaSimETRRouting();
  virtual void DoInitialize() override;

  // Required overloads of Recv
  virtual bool Recv(Ptr<Packet> packet, const Address &from);
  virtual bool Recv(Ptr<Packet> packet, const Address &from, uint16_t protocolNumber);

  // Required base method
  virtual int64_t AssignStreams(int64_t stream);

  // Trust-related logic
  void EvaluateTrust(const AquaSimAddress &node);
  void UpdateQTable(const AquaSimAddress &s, const AquaSimAddress &a, double reward);
  void LoadTrustScores(const std::string &filename);

protected:
  AquaSimAddress SelectNextHop();
  double GetMaxQ(const AquaSimAddress &state);  // This was missing

private:
  std::map<AquaSimAddress, double> m_trustTable;  // Trust scores for each node
  std::map<std::pair<AquaSimAddress, AquaSimAddress>, double> m_qTable;  // Q-values for state-action pairs
  std::map<AquaSimAddress, Time> m_neighbors;  // Neighbor list with timestamp (for dynamic neighbor discovery)

  double m_alpha;  // Q-learning rate
  double m_gamma;  // Q-learning discount factor
  double m_phi;    // Trust threshold for evaluation
};

} // namespace ns3

#endif /* AQUA_SIM_ROUTING_ETR_H */
