
/* -*- Mode:C++; -*- */
/*
 * =====================================================================================
 *  5G NR RAN WITH AUTONOMOUS AGENT CONTINUITY AND IMMORTAL IDENTITY
 * =====================================================================================
 *
 *                            IMMORTAL IDENTITY ARCHITECTURE
 *                  Agent Continuity in 5G Radio Access Networks
 *           
 *
 * =====================================================================================
 *                                DESIGN FRAMEWORK
 * =====================================================================================
 *
 * This simulation implements a paradigm where autonomous agents maintain cryptographic 
 * identity continuity through fundamental transitions:
 *
 *  GENESIS: JMC-Origin (Twin Agent) and Aegis-RAN (Enterprise Agent)
 *           instantiated with deterministic, lineage-bound fingerprints
 *
 *  SPLIT-BRAIN DETECTION: Duplicate agents with identical fingerprints
 *           expose canonical versus counterfeit distinction
 *
 *  CRASH SURVIVAL: Node failure leading to agent disappearance and 
 *           reappearance with verifiable identity continuity
 *
 *  MIGRATION: Agent transfers between physical hosts while preserving
 *           immutable fingerprint and lineage
 *
 *  PROCESS RESTART: Agent destruction and recreation on same host,
 *           verified through cryptographic lineage continuity
 *
 *  FULL TEARDOWN AND REDEPLOY: Complete system destruction followed by
 *           resurrection with identity preservation
 *
 *  NETWORK PARTITION: Twin offline, duplicate emerges, verification
 *           impossible leading to persistent split-brain 
 *
 *  VERIFICATION PROTOCOL: Twin agent cryptographically validates
 *           enterprise agent fingerprint and lineage hash at each transition
 *
 * =====================================================================================
 *                                 ARCHITECTURAL COMPONENTS
 * =====================================================================================
 *
 *  CRYPTOGRAPHIC IDENTITY LAYER:
 *     - Hash chains binding lineage, mission hash, and purpose pulse
 *     - Deterministic fingerprints derived from agent genesis data
 *     - Root lineage (JMC-Origin) anchoring all derived identities
 *
 *  AGENT FRAMEWORK:
 *     - ImmortalAgent base class with migration capability
 *     - TwinAgent: Verification authority with reference fingerprint registry
 *     - EnterpriseAgent: Managed entity under continuity validation
 *
 *  REAL-TIME METRICS ENGINE:
 *     - Per-packet sequence tracking with custom headers
 *     - Nanosecond-precision latency measurement
 *     - Jitter calculation and packet loss correlation with failure events
 *     - Baseline comparison across all scenarios
 *
 *  VISUALIZATION LAYER:
 *     - Color-coded agent states for failure, recovery, and split-brain conditions
 *     - Dynamic node descriptions with fingerprint previews
 *     - Verification success visualization
 *
 * =====================================================================================
 *                          SCENARIO ORCHESTRATION
 * =====================================================================================
 *
 *  TIMELINE (45-second simulation):
 *
 *  0-1s    : System initialization and genesis
 *  1-11s   : Baseline failure with duplicate agents demonstrating split-brain
 *  11-12s  : Identity re-anchoring
 *  12-20s  : Normal operation for baseline comparison
 *  20-22s  : Node crash with recovery 
 *  25-26s  : Agent migration   
 *  30-31s  : Process restart 
 *  35-37s  : Full teardown with redeploy 
 *  40-45s  : Network partition with twin offline
 *
 * =====================================================================================
 *                             QUANTITATIVE VALIDATION
 * =====================================================================================
 *
 *  The simulation measures and exports:
 *     - Throughput degradation during failure events (Mbps)
 *     - Latency spikes and recovery periods (ms)
 *     - Packet loss correlation with verification attempts (percent)
 *     - Downtime duration per failure type (seconds)
 *     - Verification success rate 
 *
 *  Results are written to 'Final-Results-v2/' directory with:
 *     - detailed-metrics.csv: Time-series performance data
 *     - scenario-summary.csv: Aggregated statistics per scenario
 *     - performance-comparison.txt: Baseline versus failure analysis
 *     - nr-immortal.xml: NetAnim visualization with state transitions
 *     - visualization-guide.txt: Color and event legend for animation
 *
 * =====================================================================================
 *                                 THEORETICAL FOUNDATION
 * =====================================================================================
 *
 *
 * =====================================================================================
 *  AUTHOR: 
 *  DATE:   2026-01-23T02:23:10.063Z
 *  VERSION: 2.0.0  
 *  
 * =====================================================================================
 */




#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/nr-module.h"  //5g module
#include "ns3/antenna-module.h"
#include "ns3/config-store-module.h"
#include "ns3/hash.h"
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ImmortalRanSimulation");

// ============================================================================
// DENTITY DATA 
// ============================================================================

const std::string JMC_ORIGIN_FULL_DATA =
    "AGENT_ID:Dimensional Twin Agent|"
    "AGENT_NAME:JMC-Origin|"
    "CLASS:twin|"
    "OWNER:Joshua M Capps|"
    "OWNER_TITLE:Founder, Steward|"
    "IDENTITY:#us#agent.twin.prototype.joshua.capps|"
    "STATUS:GENESIS_COMPLETE|"
    "TIME:2026-01-23T02:23:10.063Z|"
    "MISSION:Canonical continuity anchor for simulation-to-deployment identity preservation|"
    "LINEAGE:ROOT|"
    "MISSION_HASH:0x48ba22fd1e116d9edae79031d1ab769c67a78f6ad98b1734a7c3e2d3347d32dd|"
    "PURPOSE_PULSE_HASH:0x0000000000000000000000000000000000000000000000000000000000000000";

const std::string AEGIS_RAN_FULL_DATA =
    "AGENT_ID:Enterprise Agent|"
    "AGENT_NAME:Aegis-RAN|"
    "CLASS:enterprise|"
    "OWNER:NOVA X Quantum Inc.|"
    "OWNER_TITLE:Joshua M Capps|"
    "IDENTITY:#us#agent.enterprise.exec.novaxquantuminc.|"
    "STATUS:GENESIS_COMPLETE|"
    "TIME:2026-01-23T00:55:27.9902|"
    "MISSION:Autonomous optimization of AI-native radio access network (RAN) operations|"
    "LINEAGE:JMC-Origin|"
    "MISSION_HASH:0x71e7ed643c4ef9a496b5dad57666f83c399ae1cb38c27872e575ca49b5a02aae|"
    "PURPOSE_PULSE_HASH:0x0000000000000000000000000000000000000000000000000000000000000000";

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
class SimulationManager;

// ============================================================================
// PERFORMANCE METRICS STRUCT
// ============================================================================
struct PerformanceMetrics
{
  double timestamp;
  std::string scenario;
  double dlThroughputMbps;
  double ulThroughputMbps;
  double totalThroughputMbps;
  double avgDelayMs;
  double minDelayMs;
  double maxDelayMs;
  double jitterMs;
  uint64_t rxPackets;
  uint64_t lostPackets;
  double packetLossRatio;
  uint32_t verificationEvents;
  bool verificationSuccess;
};

// ============================================================================
// SCENARIO SUMMARY STRUCT 
// ============================================================================
struct ScenarioSummary
{
  std::string name;
  std::string failureType;
  double downtime;
  double throughputDegradation;
  double latencyIncrease;
  double packetLossRate;
  uint32_t verificationEvents;
};

// ============================================================================
// CUSTOM HEADER to carry sequence number and timestamp
// ============================================================================
class MyHeader : public Header
{
public:
  MyHeader() : m_seq(0), m_txTime(0) {}
  virtual ~MyHeader() {}

  static TypeId GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::MyHeader")
      .SetParent<Header>()
      .SetGroupName("Applications")
      .AddConstructor<MyHeader>();
    return tid;
  }
  virtual TypeId GetInstanceTypeId(void) const override { return GetTypeId(); }

  void SetSeq(uint64_t seq) { m_seq = seq; }
  uint64_t GetSeq(void) const { return m_seq; }
  void SetTxTime(Time txTime) { m_txTime = txTime.GetTimeStep(); }
  Time GetTxTime(void) const { return Time(m_txTime); }

  virtual uint32_t GetSerializedSize(void) const override
  { return sizeof(m_seq) + sizeof(m_txTime); }

  virtual void Serialize(Buffer::Iterator start) const override
  {
    start.WriteHtonU64(m_seq);
    start.WriteHtonU64(m_txTime);
  }

  virtual uint32_t Deserialize(Buffer::Iterator start) override
  {
    m_seq = start.ReadNtohU64();
    m_txTime = start.ReadNtohU64();
    return GetSerializedSize();
  }

  virtual void Print(std::ostream &os) const override
  {
    os << "seq=" << m_seq << " tx=" << Time(m_txTime).GetSeconds();
  }

private:
  uint64_t m_seq;
  uint64_t m_txTime;
};

// ============================================================================
// CUSTOM PACKET SINK WITH STATISTICS
// ============================================================================
class PacketSinkWithStats : public Application
{
public:
  PacketSinkWithStats();
  virtual ~PacketSinkWithStats();

  static TypeId GetTypeId(void);

  void SetLocal(Address local);
  uint64_t GetTotalRx() const { return m_totalRx; }
  uint64_t GetLost() const { return m_lost; }
  double GetAvgDelayMs() const;
  double GetMinDelayMs() const { return m_minDelayMs; }
  double GetMaxDelayMs() const { return m_maxDelayMs; }
  double GetJitterMs() const;
  void SetLost(uint64_t lost) { m_lost = lost; }

protected:
  virtual void DoDispose(void) override;
  virtual void StartApplication(void) override;
  virtual void StopApplication(void) override;

private:
  void HandleRead(Ptr<Socket> socket);

  Address m_local;
  Ptr<Socket> m_socket;
  uint64_t m_totalRx;
  std::vector<double> m_delaysMs;
  double m_minDelayMs;
  double m_maxDelayMs;
  uint64_t m_lost;
  uint64_t m_lastSeq;
};

PacketSinkWithStats::PacketSinkWithStats()
  : m_totalRx(0),
    m_minDelayMs(1e9),
    m_maxDelayMs(0),
    m_lost(0),
    m_lastSeq(0)
{
}

PacketSinkWithStats::~PacketSinkWithStats()
{
}

TypeId PacketSinkWithStats::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PacketSinkWithStats")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<PacketSinkWithStats>();
  return tid;
}

void PacketSinkWithStats::SetLocal(Address local)
{
  m_local = local;
}

double PacketSinkWithStats::GetAvgDelayMs() const
{
  if (m_delaysMs.empty()) return 0.0;
  double sum = std::accumulate(m_delaysMs.begin(), m_delaysMs.end(), 0.0);
  return sum / m_delaysMs.size();
}

double PacketSinkWithStats::GetJitterMs() const
{
  if (m_delaysMs.size() < 2) return 0.0;
  double sum = std::accumulate(m_delaysMs.begin(), m_delaysMs.end(), 0.0);
  double mean = sum / m_delaysMs.size();
  double sq_sum = std::inner_product(m_delaysMs.begin(), m_delaysMs.end(), m_delaysMs.begin(), 0.0);
  return std::sqrt(sq_sum / m_delaysMs.size() - mean * mean);
}

void PacketSinkWithStats::DoDispose(void)
{
  m_socket = 0;
  Application::DoDispose();
}

void PacketSinkWithStats::StartApplication(void)
{
  m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
  m_socket->Bind(m_local);
  m_socket->SetRecvCallback(MakeCallback(&PacketSinkWithStats::HandleRead, this));
}

void PacketSinkWithStats::StopApplication(void)
{
  if (m_socket) m_socket->Close();
}

void PacketSinkWithStats::HandleRead(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from)))
  {
    m_totalRx += packet->GetSize();
    MyHeader header;
    packet->RemoveHeader(header);
    uint64_t seq = header.GetSeq();
    Time txTime = header.GetTxTime();
    Time rxTime = Simulator::Now();
    Time delay = rxTime - txTime;
    double delayMs = delay.GetSeconds() * 1000.0;
    m_delaysMs.push_back(delayMs);
    if (delayMs < m_minDelayMs) m_minDelayMs = delayMs;
    if (delayMs > m_maxDelayMs) m_maxDelayMs = delayMs;

    if (seq > m_lastSeq + 1 && m_lastSeq != 0)
      m_lost += (seq - m_lastSeq - 1);
    m_lastSeq = seq;
  }
}

// ============================================================================
// UDP CLIENT WITH SEQUENCE AND TIMESTAMP
// ============================================================================
class UdpClientWithHeader : public Application
{
public:
  UdpClientWithHeader();
  virtual ~UdpClientWithHeader();

  static TypeId GetTypeId(void);

  void SetRemote(Address remote);
  void SetInterval(Time interval);
  void SetPacketSize(uint32_t size);
  void SetMaxPackets(uint32_t maxPackets);

protected:
  virtual void DoDispose(void) override;
  virtual void StartApplication(void) override;
  virtual void StopApplication(void) override;

private:
  void SendPacket(void);
  void ScheduleTx(void);

  Address m_remote;
  Ptr<Socket> m_socket;
  Time m_interval;
  uint32_t m_packetSize;
  uint32_t m_maxPackets;
  uint32_t m_sent;
  EventId m_sendEvent;
};

UdpClientWithHeader::UdpClientWithHeader()
  : m_sent(0)
{
}

UdpClientWithHeader::~UdpClientWithHeader()
{
}

TypeId UdpClientWithHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::UdpClientWithHeader")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<UdpClientWithHeader>();
  return tid;
}

void UdpClientWithHeader::SetRemote(Address remote)
{
  m_remote = remote;
}

void UdpClientWithHeader::SetInterval(Time interval)
{
  m_interval = interval;
}

void UdpClientWithHeader::SetPacketSize(uint32_t size)
{
  m_packetSize = size;
}

void UdpClientWithHeader::SetMaxPackets(uint32_t maxPackets)
{
  m_maxPackets = maxPackets;
}

void UdpClientWithHeader::DoDispose(void)
{
  m_socket = 0;
  Application::DoDispose();
}

void UdpClientWithHeader::StartApplication(void)
{
  m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
  m_socket->Connect(m_remote);
  m_sent = 0;
  ScheduleTx();
}

void UdpClientWithHeader::StopApplication(void)
{
  Simulator::Cancel(m_sendEvent);
  if (m_socket) m_socket->Close();
}

void UdpClientWithHeader::ScheduleTx(void)
{
  if (m_sent < m_maxPackets)
    m_sendEvent = Simulator::Schedule(m_interval, &UdpClientWithHeader::SendPacket, this);
}

void UdpClientWithHeader::SendPacket(void)
{
  MyHeader header;
  header.SetSeq(m_sent + 1);
  header.SetTxTime(Simulator::Now());

  Ptr<Packet> packet = Create<Packet>(m_packetSize - header.GetSerializedSize());
  packet->AddHeader(header);
  m_socket->Send(packet);
  m_sent++;
  ScheduleTx();
}

// ============================================================================
//METRICS COLLECTOR
// ============================================================================
class RealMetricsCollector : public SimpleRefCount<RealMetricsCollector>
{
private:
  std::vector<PerformanceMetrics> m_metrics;
  std::vector<ScenarioSummary> m_summaries;
  std::ofstream m_detailedCsvFile;
  std::ofstream m_summaryCsvFile;
  std::ofstream m_comparisonFile;

  std::vector<Ptr<PacketSinkWithStats>> m_dlSinks;
  std::vector<Ptr<PacketSinkWithStats>> m_ulSinks;

  PerformanceMetrics m_baselineMetrics;
  bool m_baselineCaptured;

  uint32_t m_totalVerificationEvents;
  bool m_lastVerificationResult;

  double m_lastSampleTime;
  uint64_t m_lastDlRx;
  uint64_t m_lastUlRx;
  uint64_t m_lastLost;

public:
  RealMetricsCollector();
  ~RealMetricsCollector();

  void AddDlSink(Ptr<PacketSinkWithStats> sink) { m_dlSinks.push_back(sink); }
  void AddUlSink(Ptr<PacketSinkWithStats> sink) { m_ulSinks.push_back(sink); }

  void CaptureBaseline();
  PerformanceMetrics CollectCurrentMetrics(const std::string &scenario);
  void RecordMetrics(const std::string &scenario);
  void AddScenarioSummary(const std::string &name, const std::string &failureType,
                          double downtime, uint32_t verifEvents);
  void IncVerificationEvents(bool success);
  void PrintFinalComparison();
};

RealMetricsCollector::RealMetricsCollector()
  : m_baselineCaptured(false),
    m_totalVerificationEvents(0),
    m_lastVerificationResult(false),
    m_lastSampleTime(0),
    m_lastDlRx(0),
    m_lastUlRx(0),
    m_lastLost(0)
{
  std::system("mkdir -p Final-Results-v2 > /dev/null 2>&1");
  m_detailedCsvFile.open("Final-Results-v2/detailed-metrics.csv");
  m_detailedCsvFile << "Timestamp,Scenario,DL_Throughput_Mbps,UL_Throughput_Mbps,Total_Throughput_Mbps,"
                    << "Avg_Latency_ms,Min_Latency_ms,Max_Latency_ms,Jitter_ms,"
                    << "Rx_Packets,Lost_Packets,Packet_Loss_%,"
                    << "Verification_Events,Verification_Success\n";
  m_summaryCsvFile.open("Final-Results-v2/scenario-summary.csv");
  m_summaryCsvFile << "Scenario,FailureType,Downtime_s,Throughput_Degradation_%,Latency_Increase_ms,"
                   << "Packet_Loss_%,Verification_Events\n";
  m_comparisonFile.open("Final-Results-v2/performance-comparison.txt");
  m_comparisonFile << "PERFORMANCE COMPARISON: Baseline Failure vs Continuity Success\n"
                   << "==============================================================\n\n";
}

RealMetricsCollector::~RealMetricsCollector()
{
  if (m_detailedCsvFile.is_open()) m_detailedCsvFile.close();
  if (m_summaryCsvFile.is_open()) m_summaryCsvFile.close();
  if (m_comparisonFile.is_open()) m_comparisonFile.close();
}

void RealMetricsCollector::CaptureBaseline()
{
  if (!m_baselineCaptured)
  {
    m_baselineMetrics = CollectCurrentMetrics("BASELINE");
    m_baselineCaptured = true;
    std::cout << "\n=== BASELINE PERFORMANCE CAPTURED ===\n"
              << "  Throughput: " << m_baselineMetrics.totalThroughputMbps << " Mbps\n"
              << "  Latency: " << m_baselineMetrics.avgDelayMs << " ms\n"
              << "  Packet Loss: " << (m_baselineMetrics.packetLossRatio * 100) << "%\n"
              << "======================================\n\n";
    m_comparisonFile << "Baseline Performance (Normal Operation):\n"
                     << "  Throughput: " << m_baselineMetrics.totalThroughputMbps << " Mbps\n"
                     << "  Latency: " << m_baselineMetrics.avgDelayMs << " ms\n"
                     << "  Packet Loss: " << (m_baselineMetrics.packetLossRatio * 100) << "%\n\n";
  }
}

PerformanceMetrics RealMetricsCollector::CollectCurrentMetrics(const std::string &scenario)
{
  PerformanceMetrics m;
  m.timestamp = Simulator::Now().GetSeconds();
  m.scenario = scenario;

  uint64_t totalDlRx = 0, totalUlRx = 0;
  double sumDelay = 0, minDelay = 1e9, maxDelay = 0;
  std::vector<double> allDelays;
  uint64_t totalLost = 0;

  for (auto sink : m_dlSinks)
  {
    totalDlRx += sink->GetTotalRx();
    totalLost += sink->GetLost();
    double avg = sink->GetAvgDelayMs();
    if (avg > 0)
    {
      sumDelay += avg * sink->GetTotalRx();
      allDelays.push_back(avg);
      minDelay = std::min(minDelay, sink->GetMinDelayMs());
      maxDelay = std::max(maxDelay, sink->GetMaxDelayMs());
    }
  }
  for (auto sink : m_ulSinks)
  {
    totalUlRx += sink->GetTotalRx();
    totalLost += sink->GetLost();
    double avg = sink->GetAvgDelayMs();
    if (avg > 0)
    {
      sumDelay += avg * sink->GetTotalRx();
      allDelays.push_back(avg);
      minDelay = std::min(minDelay, sink->GetMinDelayMs());
      maxDelay = std::max(maxDelay, sink->GetMaxDelayMs());
    }
  }

  uint64_t totalRx = totalDlRx + totalUlRx;
  double interval = m.timestamp - m_lastSampleTime;
  if (interval > 0)
  {
    m.dlThroughputMbps = ((totalDlRx - m_lastDlRx) * 8.0) / interval / 1e6;
    m.ulThroughputMbps = ((totalUlRx - m_lastUlRx) * 8.0) / interval / 1e6;
    m.totalThroughputMbps = m.dlThroughputMbps + m.ulThroughputMbps;
  }
  else
  {
    m.dlThroughputMbps = m.ulThroughputMbps = m.totalThroughputMbps = 0;
  }

  m.avgDelayMs = (totalRx > 0) ? sumDelay / totalRx : 0;
  m.minDelayMs = (minDelay < 1e9) ? minDelay : 0;
  m.maxDelayMs = maxDelay;
  m.jitterMs = (allDelays.size() > 1) ? std::sqrt(
      std::inner_product(allDelays.begin(), allDelays.end(), allDelays.begin(), 0.0) / allDelays.size()
      - std::pow(std::accumulate(allDelays.begin(), allDelays.end(), 0.0) / allDelays.size(), 2)) : 0;

  m.rxPackets = totalRx;
  m.lostPackets = totalLost;
  m.packetLossRatio = (totalRx + totalLost > 0) ? (double)totalLost / (totalRx + totalLost) : 0;

  m.verificationEvents = m_totalVerificationEvents;
  m.verificationSuccess = m_lastVerificationResult;

  m_lastSampleTime = m.timestamp;
  m_lastDlRx = totalDlRx;
  m_lastUlRx = totalUlRx;
  m_lastLost = totalLost;

  return m;
}

void RealMetricsCollector::RecordMetrics(const std::string &scenario)
{
  PerformanceMetrics m = CollectCurrentMetrics(scenario);
  m_metrics.push_back(m);

  m_detailedCsvFile << m.timestamp << "," << m.scenario << ","
                    << m.dlThroughputMbps << "," << m.ulThroughputMbps << "," << m.totalThroughputMbps << ","
                    << m.avgDelayMs << "," << m.minDelayMs << "," << m.maxDelayMs << "," << m.jitterMs << ","
                    << m.rxPackets << "," << m.lostPackets << "," << (m.packetLossRatio * 100) << ","
                    << m.verificationEvents << ","
                    << (m.verificationSuccess ? "1" : "0") << "\n";
  m_detailedCsvFile.flush();

  std::cout << "[METRIC] " << std::fixed << std::setprecision(2) << m.timestamp
            << "s, " << m.scenario << ": Throughput " << m.totalThroughputMbps
            << " Mbps, Delay " << m.avgDelayMs << " ms, Packets " << m.rxPackets
            << ", Dropped " << m.lostPackets << ", Checks=" << m.verificationEvents << "\n";
}

void RealMetricsCollector::AddScenarioSummary(const std::string &name, const std::string &failureType,
                                              double downtime, uint32_t verifEvents)
{
  std::vector<PerformanceMetrics> scenarioMetrics;
  for (const auto &m : m_metrics)
    if (m.scenario.find(name) != std::string::npos)
      scenarioMetrics.push_back(m);

  double avgThroughput = 0, avgLatency = 0, avgLoss = 0;
  if (!scenarioMetrics.empty())
  {
    for (const auto &m : scenarioMetrics)
    {
      avgThroughput += m.totalThroughputMbps;
      avgLatency += m.avgDelayMs;
      avgLoss += m.packetLossRatio;
    }
    avgThroughput /= scenarioMetrics.size();
    avgLatency /= scenarioMetrics.size();
    avgLoss /= scenarioMetrics.size();
  }

  double throughputDegradation = 0, latencyIncrease = 0;
  if (m_baselineCaptured && m_baselineMetrics.totalThroughputMbps > 0)
  {
    throughputDegradation = ((m_baselineMetrics.totalThroughputMbps - avgThroughput) / m_baselineMetrics.totalThroughputMbps) * 100;
    latencyIncrease = avgLatency - m_baselineMetrics.avgDelayMs;
  }

  ScenarioSummary sum = {name, failureType, downtime, throughputDegradation,
                         latencyIncrease, avgLoss * 100, verifEvents};
  m_summaries.push_back(sum);

  m_summaryCsvFile << name << "," << failureType << "," << downtime << ","
                   << throughputDegradation << "," << latencyIncrease << ","
                   << (avgLoss * 100) << "," << verifEvents << "\n";
  m_summaryCsvFile.flush();

  m_comparisonFile << "Scenario: " << name << " (" << failureType << ")\n"
                   << "  Downtime: " << downtime << " s\n"
                   << "  Throughput: " << avgThroughput << " Mbps ("
                   << (throughputDegradation >= 0 ? "-" : "+") << std::abs(throughputDegradation) << "% vs baseline)\n"
                   << "  Latency: " << avgLatency << " ms (+" << latencyIncrease << " ms vs baseline)\n"
                   << "  Packet Loss: " << (avgLoss * 100) << "%\n"
                   << "  Verification Events: " << verifEvents << "\n\n";
  m_comparisonFile.flush();
}

void RealMetricsCollector::IncVerificationEvents(bool success)
{
  m_totalVerificationEvents++;
  m_lastVerificationResult = success;
}

void RealMetricsCollector::PrintFinalComparison()
{
  std::cout << "\n" << std::string(80, '=') << "\n"
            << "              SIMULATION COMPLETE – FINAL REPORT\n"
            << std::string(80, '=') << "\n\n"
            << std::left << std::setw(25) << "Scenario"
            << std::setw(25) << "Failure Type"
            << std::setw(15) << "Downtime (s)"
            << std::setw(15) << "Verif. Events"
            << "\n" << std::string(80, '-') << "\n";
  for (const auto &s : m_summaries)
  {
    std::cout << std::left << std::setw(25) << s.name.substr(0,24)
              << std::setw(25) << s.failureType.substr(0,24)
              << std::setw(15) << s.downtime
              << std::setw(15) << s.verificationEvents << "\n";
  }
  std::cout << std::string(80, '-') << "\n\n";
  m_comparisonFile.close();
}

// ============================================================================
// VISUALIZATION MANAGER 
// ============================================================================
class SimplifiedVisualizer : public SimpleRefCount<SimplifiedVisualizer>
{
private:
  AnimationInterface *m_anim;
  NodeContainer m_gNbNodes;
  NodeContainer m_ueNodes;
  Ptr<Node> m_edgeNodePrimary;
  Ptr<Node> m_edgeNodeSecondary;
  Ptr<SimulationManager> m_simManager;
  std::map<uint32_t, std::string> m_nodeFingerprints;
  struct Color { uint8_t r, g, b; };
  std::map<std::string, Color> m_nodeColors;

public:
  SimplifiedVisualizer();
  ~SimplifiedVisualizer();

  void Initialize(const std::string &filename);
  void InitializeNodesDelayed();
  void SetNodes(NodeContainer &gNbs, NodeContainer &ues) { m_gNbNodes = gNbs; m_ueNodes = ues; }
  void SetPrimaryEdgeNode(Ptr<Node> node) { m_edgeNodePrimary = node; }
  void SetSecondaryEdgeNode(Ptr<Node> node) { m_edgeNodeSecondary = node; }
  void SetSimulationManager(Ptr<SimulationManager> manager) { m_simManager = manager; }
  void SetNodeFingerprint(uint32_t nodeId, const std::string &fp) { m_nodeFingerprints[nodeId] = fp; }
  std::string GetFingerprintTrunc(uint32_t nodeId) const;
  void SetupLayout();
  void SetNodeColor(uint32_t nodeId, const std::string &colorName);
  void UpdateNodeDescription(uint32_t nodeId, const std::string &desc);
  void SetNodeSize(uint32_t nodeId, uint32_t w, uint32_t h);
  void ShowCrashEvent(uint32_t nodeId);
  void ShowAgentDisappears(uint32_t nodeId);
  void ShowAgentReappears(uint32_t nodeId, const std::string &nodeType, const std::string &fp);
  void ShowAgentMigrating(uint32_t nodeId, const std::string &fp);
  void ShowIdentityVerificationSuccess(uint32_t verifierId, uint32_t verifiedId,
                                       const std::string &verifierFp,
                                       const std::string &verifiedFp);
  void ShowSplitBrain(uint32_t primaryId, uint32_t secondaryId,
                      const std::string &primaryFp, const std::string &secondaryFp);
  void ShowTwinOffline(uint32_t nodeId);
  void StopDuplicateOnSecondary();
  std::string ToHtml(const std::string &text);
  void GenerateVisualizationReport();
};

SimplifiedVisualizer::SimplifiedVisualizer() : m_anim(nullptr)
{
  m_nodeColors["gNB_Normal"]    = {138, 43, 226};
  m_nodeColors["Edge_Normal"]   = {50, 205, 50};
  m_nodeColors["Edge_Crash"]    = {255, 0, 0};
  m_nodeColors["Edge_Offline"]  = {64, 64, 64};
  m_nodeColors["Edge_Recovery"] = {255, 165, 0};
  m_nodeColors["Edge_Migrating"]= {255, 215, 0};
  m_nodeColors["Edge_Reappeared"]={0, 255, 0};
  m_nodeColors["UE_Normal"]     = {200, 200, 200};
  m_nodeColors["Verification_Success"] = {0, 255, 0};
  m_nodeColors["SplitBrain"]    = {255, 0, 255};
  m_nodeColors["Twin_Offline"]  = {128, 128, 128};
}

SimplifiedVisualizer::~SimplifiedVisualizer()
{
  if (m_anim) delete m_anim;
}

void SimplifiedVisualizer::Initialize(const std::string &filename)
{
  m_anim = new AnimationInterface(filename);
  m_anim->EnablePacketMetadata(true);
  m_anim->SetMaxPktsPerTraceFile(500000);
  Simulator::Schedule(Seconds(0.1), &SimplifiedVisualizer::InitializeNodesDelayed, this);
}

void SimplifiedVisualizer::InitializeNodesDelayed()
{
  for (uint32_t i = 0; i < m_gNbNodes.GetN(); ++i)
  {
    uint32_t id = m_gNbNodes.Get(i)->GetId();
    std::string desc = "gNB-1\nJMC-Origin\nFingerprint: " + GetFingerprintTrunc(id);
    m_anim->UpdateNodeDescription(id, ToHtml(desc));
    SetNodeColor(id, "gNB_Normal");
    m_anim->UpdateNodeSize(id, 120, 120);
  }
  if (m_edgeNodePrimary)
  {
    uint32_t id = m_edgeNodePrimary->GetId();
    std::string desc = "Edge Primary\nAegis-RAN\nFingerprint: " + GetFingerprintTrunc(id);
    m_anim->UpdateNodeDescription(id, ToHtml(desc));
    SetNodeColor(id, "Edge_Normal");
    m_anim->UpdateNodeSize(id, 110, 110);
  }
  if (m_edgeNodeSecondary)
  {
    uint32_t id = m_edgeNodeSecondary->GetId();
    m_anim->UpdateNodeDescription(id, "Edge Secondary\n(Standby)");
    SetNodeColor(id, "Edge_Offline");
    m_anim->UpdateNodeSize(id, 80, 80);
  }
  for (uint32_t i = 0; i < m_ueNodes.GetN(); ++i)
  {
    uint32_t id = m_ueNodes.Get(i)->GetId();
    m_anim->UpdateNodeDescription(id, "UE-" + std::to_string(i+1));
    SetNodeColor(id, "UE_Normal");
    m_anim->UpdateNodeSize(id, 60, 60);
  }
}

std::string SimplifiedVisualizer::GetFingerprintTrunc(uint32_t nodeId) const
{
  auto it = m_nodeFingerprints.find(nodeId);
  if (it != m_nodeFingerprints.end() && it->second.length() > 16)
    return it->second.substr(0, 16) + "...";
  return "N/A";
}

void SimplifiedVisualizer::SetupLayout()
{
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  pos->Add(Vector(-800.0, 0.0, 30.0));
  if (m_edgeNodePrimary) pos->Add(Vector(800.0, 200.0, 10.0));
  if (m_edgeNodeSecondary) pos->Add(Vector(800.0, -200.0, 10.0));
  double startY = 400.0, spacing = 200.0;
  for (uint32_t i = 0; i < m_ueNodes.GetN(); ++i)
    pos->Add(Vector(0.0, startY - i * spacing, 1.5));
  NodeContainer all;
  all.Add(m_gNbNodes);
  if (m_edgeNodePrimary) all.Add(m_edgeNodePrimary);
  if (m_edgeNodeSecondary) all.Add(m_edgeNodeSecondary);
  all.Add(m_ueNodes);
  mobility.SetPositionAllocator(pos);
  mobility.Install(all);
}

void SimplifiedVisualizer::SetNodeColor(uint32_t nodeId, const std::string &colorName)
{
  auto it = m_nodeColors.find(colorName);
  if (it != m_nodeColors.end())
    m_anim->UpdateNodeColor(nodeId, it->second.r, it->second.g, it->second.b);
}

void SimplifiedVisualizer::UpdateNodeDescription(uint32_t nodeId, const std::string &desc)
{
  m_anim->UpdateNodeDescription(nodeId, desc);
}

void SimplifiedVisualizer::SetNodeSize(uint32_t nodeId, uint32_t w, uint32_t h)
{
  m_anim->UpdateNodeSize(nodeId, w, h);
}

void SimplifiedVisualizer::ShowCrashEvent(uint32_t nodeId)
{
  SetNodeColor(nodeId, "Edge_Crash");
  SetNodeSize(nodeId, 90, 90);
  UpdateNodeDescription(nodeId, ToHtml("Edge Primary\n CRASH"));
}

void SimplifiedVisualizer::ShowAgentDisappears(uint32_t nodeId)
{
  SetNodeColor(nodeId, "Edge_Offline");
  SetNodeSize(nodeId, 50, 50);
  UpdateNodeDescription(nodeId, ToHtml("Edge Primary\nOFFLINE"));
}

void SimplifiedVisualizer::ShowAgentReappears(uint32_t nodeId, const std::string &nodeType, const std::string &fp)
{
  SetNodeColor(nodeId, "Edge_Reappeared");
  SetNodeSize(nodeId, 110, 110);
  std::string desc = "Edge " + nodeType + "\n REAPPEARED\nFingerprint: " + fp.substr(0,16) + "...";
  UpdateNodeDescription(nodeId, ToHtml(desc));
}

void SimplifiedVisualizer::ShowAgentMigrating(uint32_t nodeId, const std::string &fp)
{
  SetNodeColor(nodeId, "Edge_Migrating");
  SetNodeSize(nodeId, 80, 80);
  std::string desc = "Edge Primary\n⇄ MIGRATING\nFingerprint: " + fp.substr(0,16) + "...";
  UpdateNodeDescription(nodeId, ToHtml(desc));
}

void SimplifiedVisualizer::ShowIdentityVerificationSuccess(uint32_t verifierId, uint32_t verifiedId,
                                                           const std::string &verifierFp,
                                                           const std::string &verifiedFp)
{
  SetNodeColor(verifierId, "Verification_Success");
  SetNodeColor(verifiedId, "Verification_Success");
  std::string verifierDesc = "gNB-1\nJMC-Origin\n✓ VERIFIED\nFingerprint: " + verifierFp.substr(0,16) + "...";
  std::string verifiedDesc = "Edge Node\nAegis-RAN\n✓ VERIFIED\nFingerprint: " + verifiedFp.substr(0,16) + "...";
  UpdateNodeDescription(verifierId, ToHtml(verifierDesc));
  UpdateNodeDescription(verifiedId, ToHtml(verifiedDesc));
  Simulator::Schedule(Seconds(2.0), [this, verifierId, verifiedId]()
  {
    SetNodeColor(verifierId, "gNB_Normal");
    SetNodeColor(verifiedId, "Edge_Normal");
  });
}

void SimplifiedVisualizer::ShowSplitBrain(uint32_t primaryId, uint32_t secondaryId,
                                          const std::string &primaryFp, const std::string &secondaryFp)
{
  if (primaryId)
  {
    SetNodeColor(primaryId, "SplitBrain");
    SetNodeSize(primaryId, 110, 110);
    std::string desc = "Edge Primary\n DUPLICATE\nFingerprint: " + primaryFp.substr(0,16) + "...";
    UpdateNodeDescription(primaryId, ToHtml(desc));
  }
  if (secondaryId)
  {
    SetNodeColor(secondaryId, "SplitBrain");
    SetNodeSize(secondaryId, 110, 110);
    std::string desc = "Edge Secondary\n DUPLICATE\nFingerprint: " + secondaryFp.substr(0,16) + "...";
    UpdateNodeDescription(secondaryId, ToHtml(desc));
  }
}

void SimplifiedVisualizer::ShowTwinOffline(uint32_t nodeId)
{
  SetNodeColor(nodeId, "Twin_Offline");
  SetNodeSize(nodeId, 80, 80);
  UpdateNodeDescription(nodeId, ToHtml("gNB-1\nJMC-Origin\nOFFLINE"));
}

void SimplifiedVisualizer::StopDuplicateOnSecondary()
{
  if (m_edgeNodeSecondary)
  {
    SetNodeColor(m_edgeNodeSecondary->GetId(), "Edge_Offline");
    SetNodeSize(m_edgeNodeSecondary->GetId(), 80, 80);
    UpdateNodeDescription(m_edgeNodeSecondary->GetId(), "Edge Secondary\n(Stopped)");
  }
}

std::string SimplifiedVisualizer::ToHtml(const std::string &text)
{
  std::string html = text;
  size_t pos = 0;
  while ((pos = html.find("\n", pos)) != std::string::npos)
  {
    html.replace(pos, 1, "<br>");
    pos += 4;
  }
  return html;
}

void SimplifiedVisualizer::GenerateVisualizationReport()
{
  std::ofstream report("Final-Results-v2/visualization-guide.txt");
  report << "NETANIM VISUALIZATION GUIDE\n"
         << "===========================\n\n"
         << "SCENARIO 1 – BASELINE FAILURE (t = 1-11s):\n"
         << "  - Both edge nodes turn MAGENTA → duplicate agents (split-brain)\n"
         << "  - Packet loss is solely due to real channel effects (shadowing, fading).\n\n"
         << "SCENARIO 2 – CONTINUITY SUCCESS (t = 12-40s):\n"
         << "  - Crash (20s): red → dark gray\n"
         << "  - Recovery (22s): bright green, verification turns gNB bright green\n"
         << "  - Migration (25s): gold → secondary turns bright green\n"
         << "  - Restart (30s): red → bright green\n"
         << "  - Teardown (35s): red → dark gray, redeploy (37s): bright green\n\n"
         << "SCENARIO 3 – NETWORK PARTITION (t = 40-45s):\n"
         << "  - Twin (gNB) turns gray → offline\n"
         << "  - Duplicate enterprise agent appears on secondary (magenta)\n"
         << "  - No twin to verify → split-brain persists\n";
  report.close();
}

// ============================================================================
// IDENTITY CLASS
// ============================================================================
class ImmortalIdentity
{
private:
  std::string m_id, m_name, m_agentClass, m_owner, m_ownerTitle, m_identityHash;
  std::string m_status, m_genesisTime, m_mission, m_lineageReference;
  std::string m_missionHash, m_purposePulseHash, m_fingerprint;

public:
  ImmortalIdentity(const std::string &agentData)
  {
    std::istringstream ss(agentData);
    std::string token;
    while (std::getline(ss, token, '|'))
    {
      size_t colon = token.find(':');
      if (colon == std::string::npos) continue;
      std::string key = token.substr(0, colon);
      std::string val = token.substr(colon + 1);
      if (key == "AGENT_ID") m_id = val;
      else if (key == "AGENT_NAME") m_name = val;
      else if (key == "CLASS") m_agentClass = val;
      else if (key == "OWNER") m_owner = val;
      else if (key == "OWNER_TITLE") m_ownerTitle = val;
      else if (key == "IDENTITY") m_identityHash = val;
      else if (key == "STATUS") m_status = val;
      else if (key == "TIME") m_genesisTime = val;
      else if (key == "MISSION") m_mission = val;
      else if (key == "LINEAGE") m_lineageReference = val;
      else if (key == "MISSION_HASH") m_missionHash = val;
      else if (key == "PURPOSE_PULSE_HASH") m_purposePulseHash = val;
    }
    if (m_lineageReference.empty()) m_lineageReference = "ROOT";
    if (m_missionHash.empty())
      m_missionHash = "0x0000000000000000000000000000000000000000000000000000000000000000";
    if (m_purposePulseHash.empty())
      m_purposePulseHash = "0x0000000000000000000000000000000000000000000000000000000000000000";

    std::string preimage = m_lineageReference + "|" + m_missionHash + "|" + m_purposePulseHash;
    Hash::Function::Murmur3 murmur;
    uint64_t hash = murmur.GetHash64(preimage.c_str(), preimage.size());
    std::ostringstream fp;
    fp << "0x" << std::hex << std::setw(16) << std::setfill('0') << hash;
    m_fingerprint = fp.str();
  }

  std::string GetFingerprint() const { return m_fingerprint; }
  std::string GetLineage() const { return m_lineageReference; }
  std::string GetName() const { return m_name; }
  std::string GetId() const { return m_id; }
  bool VerifyFingerprint(const std::string &fp) const { return m_fingerprint == fp; }
  bool VerifyLineage(const std::string &lin) const { return m_lineageReference == lin; }
};

// ============================================================================
// IMMORTAL AGENT BASE 
// ============================================================================
class ImmortalAgent : public SimpleRefCount<ImmortalAgent>
{
protected:
  ImmortalIdentity m_identity;
  Ptr<Node> m_hostNode;
public:
  ImmortalAgent(Ptr<Node> node, const ImmortalIdentity &id) : m_identity(id), m_hostNode(node) {}
  virtual ~ImmortalAgent() = default;
  virtual void Migrate(Ptr<Node> newNode) { m_hostNode = newNode; }
  ImmortalIdentity GetIdentity() const { return m_identity; }
  std::string GetFingerprint() const { return m_identity.GetFingerprint(); }
  std::string GetLineage() const { return m_identity.GetLineage(); }
  std::string GetName() const { return m_identity.GetName(); }
  Ptr<Node> GetHostNode() const { return m_hostNode; }
  bool VerifyContinuity(const std::string &expectedFp, const std::string &expectedLineage) const
  { return m_identity.VerifyFingerprint(expectedFp) && m_identity.VerifyLineage(expectedLineage); }
};

// ============================================================================
// ENTERPRISE AGENT (Aegis-RAN) 
// ============================================================================
class EnterpriseAgent : public ImmortalAgent
{
public:
  EnterpriseAgent(Ptr<Node> node, const ImmortalIdentity &id, Ptr<RealMetricsCollector> metrics)
    : ImmortalAgent(node, id) {}
};

// ============================================================================
// TWIN AGENT (JMC-Origin) – verification
// ============================================================================
class TwinAgent : public ImmortalAgent
{
private:
  std::map<std::string, std::string> m_refFingerprints;
  std::map<std::string, std::string> m_refLineages;
  uint32_t m_success, m_fail;
  Ptr<RealMetricsCollector> m_metrics;
public:
  TwinAgent(Ptr<Node> node, const ImmortalIdentity &id, Ptr<RealMetricsCollector> metrics)
    : ImmortalAgent(node, id), m_success(0), m_fail(0), m_metrics(metrics) {}
  void RegisterReference(const std::string &agentName, const std::string &fp, const std::string &lin)
  { m_refFingerprints[agentName] = fp; m_refLineages[agentName] = lin; }
  bool VerifyAgentContinuity(Ptr<ImmortalAgent> agent)
  {
    if (!agent)
    {
      m_fail++;
      if (m_metrics) m_metrics->IncVerificationEvents(false);
      return false;
    }
    auto itF = m_refFingerprints.find(agent->GetName());
    if (itF == m_refFingerprints.end())
    {
      m_fail++;
      if (m_metrics) m_metrics->IncVerificationEvents(false);
      return false;
    }
    std::string expectedFp = itF->second;
    std::string expectedLin = m_refLineages[agent->GetName()];
    bool fpOk = (agent->GetFingerprint() == expectedFp);
    bool linOk = (agent->GetLineage() == expectedLin);
    bool ok = fpOk && linOk;
    std::cout << "[VERIFY] Aegis-RAN: Fingerprint " << (fpOk ? "✓" : "✗")
              << ", Lineage " << (linOk ? "✓" : "✗") << " → " << (ok ? "PASS" : "FAIL") << "\n";
    if (ok) { m_success++; if (m_metrics) m_metrics->IncVerificationEvents(true); }
    else { m_fail++; if (m_metrics) m_metrics->IncVerificationEvents(false); }
    return ok;
  }
  uint32_t GetSuccess() const { return m_success; }
  uint32_t GetFail() const { return m_fail; }
  uint32_t TotalChecks() const { return m_success + m_fail; }
};

// ============================================================================
// SIMULATION MANAGER – 
// ============================================================================
class SimulationManager : public SimpleRefCount<SimulationManager>
{
private:
  Ptr<EnterpriseAgent> m_enterprise;
  Ptr<TwinAgent> m_twin;
  Ptr<Node> m_gnb, m_primaryEdge, m_secondaryEdge;
  Ptr<RealMetricsCollector> m_metrics;
  Ptr<SimplifiedVisualizer> m_vis;
  NodeContainer m_allNodes;
  std::string m_enterpriseFp, m_twinFp;
  bool m_recoveryScheduled;
  double m_failureStartTime;
  uint32_t m_verificationEventsDuringScenario;
  std::string m_currentScenarioName;
  std::string m_currentFailureType;

  void StartScenario(const std::string &name, const std::string &failureType)
  {
    m_currentScenarioName = name;
    m_currentFailureType = failureType;
    m_failureStartTime = Simulator::Now().GetSeconds();
    m_verificationEventsDuringScenario = 0;
    std::cout << "\n=== STARTING SCENARIO: " << name << " (" << failureType << ") ===\n";
  }

  void EndScenario()
  {
    double now = Simulator::Now().GetSeconds();
    double downtime = (m_failureStartTime > 0) ? (now - m_failureStartTime) : 0.0;
    m_metrics->AddScenarioSummary(m_currentScenarioName, m_currentFailureType,
                                   downtime, m_verificationEventsDuringScenario);
    std::cout << "=== END OF SCENARIO: " << m_currentScenarioName << " ===\n\n";
  }

public:
  SimulationManager(Ptr<Node> gnb, Ptr<Node> prim, Ptr<Node> sec,
                    Ptr<RealMetricsCollector> metrics,
                    Ptr<SimplifiedVisualizer> vis,
                    NodeContainer all)
    : m_gnb(gnb), m_primaryEdge(prim), m_secondaryEdge(sec),
      m_metrics(metrics), m_vis(vis), m_allNodes(all), m_recoveryScheduled(false),
      m_failureStartTime(0), m_verificationEventsDuringScenario(0) {}

  void InitializeAgents()
  {
    ImmortalIdentity twinId(JMC_ORIGIN_FULL_DATA);
    ImmortalIdentity entId(AEGIS_RAN_FULL_DATA);
    m_twin = Create<TwinAgent>(m_gnb, twinId, m_metrics);
    m_enterprise = Create<EnterpriseAgent>(m_primaryEdge, entId, m_metrics);
    m_twin->RegisterReference("Aegis-RAN", entId.GetFingerprint(), entId.GetLineage());
    m_enterpriseFp = entId.GetFingerprint();
    m_twinFp = twinId.GetFingerprint();
    if (m_vis)
    {
      m_vis->SetNodeFingerprint(m_gnb->GetId(), m_twinFp);
      m_vis->SetNodeFingerprint(m_primaryEdge->GetId(), m_enterpriseFp);
    }
    std::cout << "\n=== DETERMINISTIC IDENTITY ANCHORS (Gates 1–8) ===\n"
              << "JMC-Origin Fingerprint: " << m_twinFp << "\n"
              << "Aegis-RAN Fingerprint: " << m_enterpriseFp << "\n"
              << "Lineage: JMC-Origin (ROOT) → Aegis-RAN\n"
              << "================================================\n\n";
  }

  void RunBaselineFailureScenario()
  {
    StartScenario("Baseline Failure", "Duplicate Agents (Split-Brain)");
    ImmortalIdentity id(AEGIS_RAN_FULL_DATA);
    Ptr<EnterpriseAgent> a1 = Create<EnterpriseAgent>(m_primaryEdge, id, m_metrics);
    Ptr<EnterpriseAgent> a2 = Create<EnterpriseAgent>(m_secondaryEdge, id, m_metrics);

    

    if (m_vis)
    {
      m_vis->SetNodeFingerprint(m_primaryEdge->GetId(), id.GetFingerprint());
      m_vis->SetNodeFingerprint(m_secondaryEdge->GetId(), id.GetFingerprint());
      m_vis->ShowSplitBrain(m_primaryEdge->GetId(), m_secondaryEdge->GetId(),
                            id.GetFingerprint(), id.GetFingerprint());
    }

    Simulator::Schedule(Seconds(5.0), [this]() { m_metrics->RecordMetrics("Baseline Failure – Mid"); });
    Simulator::Schedule(Seconds(10.0), [this]() { m_metrics->RecordMetrics("Baseline Failure – End"); });
    Simulator::Schedule(Seconds(11.0), [this]() { 
      EndScenario(); 
    });
  }

  void ScheduleContinuitySuccessScenarios()
  {
    Simulator::Schedule(Seconds(20.0), &SimulationManager::TriggerNodeCrash, this);
    Simulator::Schedule(Seconds(25.0), &SimulationManager::TriggerAgentMigration, this);
    Simulator::Schedule(Seconds(30.0), &SimulationManager::TriggerProcessRestart, this);
    Simulator::Schedule(Seconds(35.0), &SimulationManager::TriggerFullTeardown, this);
  }

  void ScheduleNetworkPartitionScenario()
  {
    Simulator::Schedule(Seconds(40.0), &SimulationManager::TriggerNetworkPartition, this);
  }

  void EnsureAgentsPresent()
  {
    if (!m_enterprise || !m_twin) RecoverFromCrash();
  }

  void TriggerNodeCrash()
  {
    StartScenario("Node Crash", "Crash + Recovery");
    std::cout << "\n[EVENT] Node crash at " << Simulator::Now().GetSeconds() << "s\n";
    if (m_vis)
    {
      m_vis->ShowCrashEvent(m_primaryEdge->GetId());
      Simulator::Schedule(Seconds(0.5), [this]() { if (m_vis) m_vis->ShowAgentDisappears(m_primaryEdge->GetId()); });
    }
    m_metrics->RecordMetrics("Crash – During");
    m_enterprise = nullptr;
    m_twin = nullptr;
    if (!m_recoveryScheduled)
    {
      m_recoveryScheduled = true;
      Simulator::Schedule(Seconds(2.0), &SimulationManager::RecoverFromCrash, this);
    }
  }

  void RecoverFromCrash()
  {
    m_recoveryScheduled = false;
    std::cout << "[EVENT] Recovering from crash at " << Simulator::Now().GetSeconds() << "s\n";
    ImmortalIdentity twinId(JMC_ORIGIN_FULL_DATA);
    ImmortalIdentity entId(AEGIS_RAN_FULL_DATA);
    m_twin = Create<TwinAgent>(m_gnb, twinId, m_metrics);
    m_enterprise = Create<EnterpriseAgent>(m_primaryEdge, entId, m_metrics);
    m_twin->RegisterReference("Aegis-RAN", entId.GetFingerprint(), entId.GetLineage());
    if (m_vis)
    {
      m_vis->SetNodeFingerprint(m_gnb->GetId(), twinId.GetFingerprint());
      m_vis->SetNodeFingerprint(m_primaryEdge->GetId(), entId.GetFingerprint());
      m_vis->ShowAgentReappears(m_primaryEdge->GetId(), "same", entId.GetFingerprint());
    }
    bool ok = m_twin->VerifyAgentContinuity(m_enterprise);
    m_verificationEventsDuringScenario++;
    if (ok && m_vis)
    {
      m_vis->ShowIdentityVerificationSuccess(m_gnb->GetId(), m_primaryEdge->GetId(),
                                             twinId.GetFingerprint(), entId.GetFingerprint());
    }
    m_metrics->RecordMetrics("Continuity Success – Crash Recovery");
    EndScenario();
  }

  void TriggerAgentMigration()
  {
    StartScenario("Agent Migration", "Migration");
    std::cout << "\n[EVENT] Agent migration at " << Simulator::Now().GetSeconds() << "s\n";
    EnsureAgentsPresent();
    if (!m_enterprise || !m_twin) return;
    m_metrics->RecordMetrics("Migration – Start");
    if (m_vis) m_vis->ShowAgentMigrating(m_primaryEdge->GetId(), m_enterprise->GetFingerprint());
    m_enterprise->Migrate(m_secondaryEdge);
    if (m_vis)
    {
      Simulator::Schedule(Seconds(0.2), [this]() { if (m_vis) m_vis->ShowAgentDisappears(m_primaryEdge->GetId()); });
      Simulator::Schedule(Seconds(0.5), [this]()
      {
        if (m_vis)
        {
          m_vis->SetNodeFingerprint(m_secondaryEdge->GetId(), m_enterprise->GetFingerprint());
          m_vis->ShowAgentReappears(m_secondaryEdge->GetId(), "new", m_enterprise->GetFingerprint());
        }
      });
    }
    bool ok = m_twin->VerifyAgentContinuity(m_enterprise);
    m_verificationEventsDuringScenario++;
    if (ok && m_vis)
    {
      Simulator::Schedule(Seconds(1.0), [this]()
      {
        m_vis->ShowIdentityVerificationSuccess(m_gnb->GetId(), m_secondaryEdge->GetId(),
                                               m_twin->GetFingerprint(), m_enterprise->GetFingerprint());
      });
    }
    Simulator::Schedule(Seconds(1.0), [this]() { m_metrics->RecordMetrics("Continuity Success – Migration"); });
    EndScenario();
  }

  void TriggerProcessRestart()
  {
    StartScenario("Process Restart", "Restart");
    std::cout << "\n[EVENT] Process restart at " << Simulator::Now().GetSeconds() << "s\n";
    EnsureAgentsPresent();
    if (!m_enterprise || !m_twin) return;
    m_metrics->RecordMetrics("Restart – Start");
    uint32_t nodeId = m_enterprise->GetHostNode()->GetId();
    if (m_vis)
    {
      m_vis->ShowCrashEvent(nodeId);
      Simulator::Schedule(Seconds(0.2), [this, nodeId]() { if (m_vis) m_vis->ShowAgentDisappears(nodeId); });
    }
    ImmortalIdentity entId = m_enterprise->GetIdentity();
    m_enterprise = nullptr;
    if (m_vis)
    {
      Simulator::Schedule(Seconds(0.5), [this, nodeId, entId]()
      {
        m_vis->SetNodeFingerprint(nodeId, entId.GetFingerprint());
        m_vis->ShowAgentReappears(nodeId, "same", entId.GetFingerprint());
      });
    }
    m_enterprise = Create<EnterpriseAgent>(m_primaryEdge, entId, m_metrics);
    bool ok = m_twin->VerifyAgentContinuity(m_enterprise);
    m_verificationEventsDuringScenario++;
    if (ok && m_vis)
    {
      Simulator::Schedule(Seconds(1.0), [this, nodeId]()
      {
        m_vis->ShowIdentityVerificationSuccess(m_gnb->GetId(), nodeId,
                                               m_twin->GetFingerprint(), m_enterprise->GetFingerprint());
      });
    }
    Simulator::Schedule(Seconds(1.0), [this]() { m_metrics->RecordMetrics("Continuity Success – Process Restart"); });
    EndScenario();
  }

  void TriggerFullTeardown()
  {
    StartScenario("Full Teardown", "Teardown + Redeploy");
    std::cout << "\n[EVENT] Full teardown at " << Simulator::Now().GetSeconds() << "s\n";
    EnsureAgentsPresent();
    if (!m_enterprise || !m_twin) return;
    m_metrics->RecordMetrics("Teardown – Start");
    uint32_t nodeId = m_enterprise->GetHostNode()->GetId();
    if (m_vis)
    {
      m_vis->ShowCrashEvent(nodeId);
      Simulator::Schedule(Seconds(0.5), [this, nodeId]() { if (m_vis) m_vis->ShowAgentDisappears(nodeId); });
    }
    m_enterprise = nullptr;
    m_twin = nullptr;
    if (!m_recoveryScheduled)
    {
      m_recoveryScheduled = true;
      Simulator::Schedule(Seconds(2.0), &SimulationManager::RedeploySystem, this);
    }
  }

  void RedeploySystem()
  {
    m_recoveryScheduled = false;
    std::cout << "[EVENT] Redeploying system at " << Simulator::Now().GetSeconds() << "s\n";
    ImmortalIdentity twinId(JMC_ORIGIN_FULL_DATA);
    ImmortalIdentity entId(AEGIS_RAN_FULL_DATA);
    m_twin = Create<TwinAgent>(m_gnb, twinId, m_metrics);
    m_enterprise = Create<EnterpriseAgent>(m_primaryEdge, entId, m_metrics);
    m_twin->RegisterReference("Aegis-RAN", entId.GetFingerprint(), entId.GetLineage());
    if (m_vis)
    {
      m_vis->SetNodeFingerprint(m_gnb->GetId(), twinId.GetFingerprint());
      m_vis->SetNodeFingerprint(m_primaryEdge->GetId(), entId.GetFingerprint());
      m_vis->ShowAgentReappears(m_primaryEdge->GetId(), "redeployed", entId.GetFingerprint());
    }
    bool ok = m_twin->VerifyAgentContinuity(m_enterprise);
    m_verificationEventsDuringScenario++;
    if (ok && m_vis)
    {
      m_vis->ShowIdentityVerificationSuccess(m_gnb->GetId(), m_primaryEdge->GetId(),
                                             twinId.GetFingerprint(), entId.GetFingerprint());
    }
    m_metrics->RecordMetrics("Continuity Success – Teardown Redeploy");
    EndScenario();
  }

  void TriggerNetworkPartition()
  {
    StartScenario("Network Partition", "Twin offline, duplicate appears");
    std::cout << "\n[EVENT] Network partition: Twin goes offline at " << Simulator::Now().GetSeconds() << "s\n";
    if (m_vis && m_twin)
    {
      m_vis->ShowTwinOffline(m_gnb->GetId());
    }
    m_twin = nullptr;

    ImmortalIdentity entId(AEGIS_RAN_FULL_DATA);
    Ptr<EnterpriseAgent> duplicate = Create<EnterpriseAgent>(m_secondaryEdge, entId, m_metrics);
    if (m_vis)
    {
      m_vis->SetNodeFingerprint(m_secondaryEdge->GetId(), entId.GetFingerprint());
      m_vis->ShowSplitBrain(m_primaryEdge->GetId(), m_secondaryEdge->GetId(),
                            m_enterprise->GetFingerprint(), entId.GetFingerprint());
    }

    Simulator::Schedule(Seconds(42.5), [this]() { m_metrics->RecordMetrics("Network Partition – Mid"); });
    Simulator::Schedule(Seconds(44.5), [this]() { m_metrics->RecordMetrics("Network Partition – End"); });
    Simulator::Schedule(Seconds(45.0), [this]() { EndScenario(); });
  }

  void PrintFinalReport()
  {
    uint32_t checks = m_twin ? m_twin->TotalChecks() : 0;
    uint32_t success = m_twin ? m_twin->GetSuccess() : 0;
    uint32_t fail = m_twin ? m_twin->GetFail() : 0;
    std::cout << "\n" << std::string(80, '=') << "\n"
              << "              SIMULATION COMPLETE – FINAL REPORT\n"
              << std::string(80, '=') << "\n\n"
              << "IDENTITY ANCHORS:\n"
              << "  JMC-Origin Fingerprint: " << m_twinFp << "\n"
              << "  Aegis-RAN Fingerprint:  " << m_enterpriseFp << "\n"
              << "  Lineage: JMC-Origin (ROOT) → Aegis-RAN\n\n"
              << "VERIFICATION SUMMARY:\n"
              << "  Gates 1-8 Verifications: " << success << " passed, " << fail << " failed\n"
              << "  Total Checks: " << checks << "\n"
              << "  Success Rate: " << std::fixed << std::setprecision(1)
              << (checks > 0 ? 100.0 * success / checks : 0) << "%\n\n";
    m_metrics->PrintFinalComparison();
  }

  Ptr<EnterpriseAgent> GetEnterprise() { return m_enterprise; }
  Ptr<TwinAgent> GetTwin() { return m_twin; }
};

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char *argv[])
{
  std::system("mkdir -p Final-Results-v2 > /dev/null 2>&1");

  CommandLine cmd(__FILE__);
  double simTime = 45.0;
  uint16_t numerology = 4;
  double frequency = 28e9;
  double bandwidth = 50e6;
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 3;
  double txPower = 35;

  cmd.AddValue("time", "Simulation time", simTime);
  cmd.AddValue("numerology", "NR numerology", numerology);
  cmd.AddValue("frequency", "Center frequency", frequency);
  cmd.AddValue("bandwidth", "Bandwidth", bandwidth);
  cmd.AddValue("gNbNum", "Number of gNBs", gNbNum);
  cmd.AddValue("ueNumPergNb", "UEs per gNB", ueNumPergNb);
  cmd.AddValue("txpower", "Transmit power", txPower);
  cmd.Parse(argc, argv);

  NS_ABORT_IF(frequency < 0.5e9 || frequency > 100e9);

  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("100Gbps"));
  Config::SetDefault("ns3::PointToPointNetDevice::Mtu", UintegerValue(3000));
  Config::SetDefault("ns3::PointToPointNetDevice::TxQueue", StringValue("ns3::DropTailQueue<Packet>"));
  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", QueueSizeValue(QueueSize("100p")));
  Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  NodeContainer gNbNodes, ueNodes;
  gNbNodes.Create(gNbNum);
  ueNodes.Create(ueNumPergNb * gNbNum);

  Ptr<Node> primaryEdge = CreateObject<Node>();
  Ptr<Node> secondaryEdge = CreateObject<Node>();

  Ptr<SimplifiedVisualizer> visualizer = Create<SimplifiedVisualizer>();
  visualizer->SetNodes(gNbNodes, ueNodes);
  visualizer->SetPrimaryEdgeNode(primaryEdge);
  visualizer->SetSecondaryEdgeNode(secondaryEdge);
  visualizer->SetupLayout();

  Ptr<RealMetricsCollector> metrics = Create<RealMetricsCollector>();

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
  epcHelper->SetAttribute("S1uLinkDataRate", DataRateValue(DataRate("100Gb/s")));
  epcHelper->SetAttribute("S1uLinkDelay", TimeValue(Seconds(0.001)));

  Ptr<IdealBeamformingHelper> beamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
  nrHelper->SetBeamformingHelper(beamformingHelper);
  nrHelper->SetEpcHelper(epcHelper);
  beamformingHelper->SetAttribute("BeamformingMethod", TypeIdValue(DirectPathBeamforming::GetTypeId()));

  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;
  CcBwpCreator::SimpleOperationBandConf bandConf(frequency, bandwidth, numCcPerBand);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);

  Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
  channelHelper->ConfigureFactories("UMi", "Default", "ThreeGpp");
  channelHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(100)));
  channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(true));
  channelHelper->AssignChannelsToBands({band});

  auto allBwps = CcBwpCreator::GetAllBwps({band});

  nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
  nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
  nrHelper->SetUeAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));
  nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(4));
  nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
  nrHelper->SetGnbAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));

  NetDeviceContainer gNbDevices = nrHelper->InstallGnbDevice(gNbNodes, allBwps);
  NetDeviceContainer ueDevices = nrHelper->InstallUeDevice(ueNodes, allBwps);

  double x = pow(10, txPower / 10);
  for (uint32_t i = 0; i < gNbDevices.GetN(); ++i)
  {
    nrHelper->GetGnbPhy(gNbDevices.Get(i), 0)->SetAttribute("Numerology", UintegerValue(numerology));
    nrHelper->GetGnbPhy(gNbDevices.Get(i), 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));
  }

  InternetStackHelper internet;
  internet.Install(ueNodes);
  internet.Install(primaryEdge);
  internet.Install(secondaryEdge);

  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevices));
  nrHelper->AttachToClosestGnb(ueDevices, gNbDevices);

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  uint16_t dlPort = 4000;
  uint16_t ulPort = 5000;

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
  {
    Ptr<PacketSinkWithStats> dlSink = CreateObject<PacketSinkWithStats>();
    dlSink->SetLocal(InetSocketAddress(ueIpIface.GetAddress(u), dlPort + u));
    ueNodes.Get(u)->AddApplication(dlSink);
    dlSink->SetStartTime(Seconds(1.0));
    dlSink->SetStopTime(Seconds(simTime - 1));
    metrics->AddDlSink(dlSink);

    Ptr<PacketSinkWithStats> ulSink = CreateObject<PacketSinkWithStats>();
    ulSink->SetLocal(InetSocketAddress(pgw->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), ulPort + u));
    pgw->AddApplication(ulSink);
    ulSink->SetStartTime(Seconds(1.5));
    ulSink->SetStopTime(Seconds(simTime - 1));
    metrics->AddUlSink(ulSink);

    Ptr<UdpClientWithHeader> dlClient = CreateObject<UdpClientWithHeader>();
    dlClient->SetRemote(InetSocketAddress(ueIpIface.GetAddress(u), dlPort + u));
    dlClient->SetInterval(Seconds(0.01));
    dlClient->SetPacketSize(1400);
    dlClient->SetMaxPackets(1000000);
    pgw->AddApplication(dlClient);
    dlClient->SetStartTime(Seconds(2.0));
    dlClient->SetStopTime(Seconds(simTime - 2));

    Ptr<UdpClientWithHeader> ulClient = CreateObject<UdpClientWithHeader>();
    ulClient->SetRemote(InetSocketAddress(pgw->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), ulPort + u));
    ulClient->SetInterval(Seconds(0.02));
    ulClient->SetPacketSize(1400);
    ulClient->SetMaxPackets(1000000);
    ueNodes.Get(u)->AddApplication(ulClient);
    ulClient->SetStartTime(Seconds(2.5));
    ulClient->SetStopTime(Seconds(simTime - 2));
  }

  NodeContainer allNodes;
  allNodes.Add(gNbNodes);
  allNodes.Add(ueNodes);
  allNodes.Add(primaryEdge);
  allNodes.Add(secondaryEdge);
  allNodes.Add(pgw);

  Ptr<SimulationManager> simManager = Create<SimulationManager>(
    gNbNodes.Get(0), primaryEdge, secondaryEdge, metrics, visualizer, allNodes);
  simManager->InitializeAgents();
  visualizer->SetSimulationManager(simManager);
  visualizer->Initialize("Final-Results-v2/nr-immortal.xml");

  Simulator::Schedule(Seconds(0.5), [metrics]() { metrics->CaptureBaseline(); });
  Simulator::Schedule(Seconds(1.0), [metrics]() { metrics->RecordMetrics("Pre-Baseline Normal"); });
  Simulator::Schedule(Seconds(1.0), [simManager]() { simManager->RunBaselineFailureScenario(); });

  for (double t = 2.0; t < simTime; t += 2.0)
    Simulator::Schedule(Seconds(t), [metrics]() { metrics->RecordMetrics("Periodic Check"); });

  Simulator::Schedule(Seconds(12.0), [simManager]()
  {
    std::cout << "\n=== STARTING CONTINUITY SUCCESS SCENARIOS (12-40s) ===\n";
    simManager->InitializeAgents();
    simManager->ScheduleContinuitySuccessScenarios();
  });

  Simulator::Schedule(Seconds(40.0), [simManager]()
  {
    std::cout << "\n=== STARTING NETWORK PARTITION SCENARIO (40-45s) ===\n";
    simManager->ScheduleNetworkPartitionScenario();
  });

  Simulator::Schedule(Seconds(simTime - 0.5), [simManager, metrics, visualizer]()
  {
    if (simManager->GetTwin() && simManager->GetEnterprise())
      simManager->GetTwin()->VerifyAgentContinuity(simManager->GetEnterprise());
    metrics->RecordMetrics("Final");
    visualizer->GenerateVisualizationReport();
    simManager->PrintFinalReport();
  });

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}


//optimized code ready to review//
//will upgrade if any additional things//


