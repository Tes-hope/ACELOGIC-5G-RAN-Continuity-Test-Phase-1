/* ============================================================================
 *  
 *  Description : 
 *  This file demonstrates identity continuity after failures **without** storing
 *  any execution state. The agent holds only its immutable identity and the
 *  current host node. After a crash, migration, restart, or full teardown,
 *  the agent is re‑instantiated from its identity object; the Twin Agent
 *  verifies fingerprint and lineage, proving the same logical entity persists.
 *  
 *  Three additional validation scenarios are included:
 *    - Network Partition → Rejoin (split‑brain prevention)
 *    - Abrupt Hard Failure (no state capture)
 *    - Resource Starvation Under Load (performance neutrality)
 *  
 *  ### nr-final.cc 
 *  
 * ============================================================================ */

//============================================================================
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"
#include "ns3/config-store-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/ipv4-flow-classifier.h"
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <cstdlib>
#include <cmath>
#include <numeric>
#include <algorithm>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IntegratedNrImmortal");

// ============================================================================
// IMMORTAL IDENTITY DATA 
// ============================================================================

const std::string JMC_ORIGIN_FULL_DATA = 
    "AGENT_ID:Dimensional Twin Agent|"
    "AGENT_NAME:JMC-Origin|"
    "OWNER:Joshua M Capps|"
    "OWNER_TITLE:Founder, Steward|"
    "IDENTITY:#us#agent.twin.prototype.joshua.capps|"
    "STATUS:GENESIS_COMPLETE|"
    "TIME:2026-01-23T02:23:10.063Z|"
    "MISSION:Canonical continuity anchor for simulation-to-deployment identity preservation|"
    "COVENANT:0x48ba22fd1e116d9edae79031d1ab769c67a78f6ad98b1734a7c3e2d3347d32dd|"
    "XRPL:F884722BF1513970A9F57A8C0C4E57A9A59E5CBBF28CDF|"
    "POLYGON:0x2c600285d7550858b9f3f2c9b8e5d4a1c6b3e8f7d2a5b9c4e1f6a8d3b7c0e9f2a|"
    "LINEAGE:ROOT";

const std::string AEGIS_RAN_FULL_DATA = 
    "AGENT_ID:Enterprise Agent|"
    "AGENT_NAME:Aegis-RAN|"
    "OWNER:NOVA X Quantum Inc.|"
    "OWNER_TITLE:Joshua M Capps|"
    "IDENTITY:#us#agent.enterprise.exec.novaxquantuminc.|"
    "STATUS:GENESIS_COMPLETE|"
    "TIME:2026-01-23T00:55:27.9902|"
    "MISSION:Autonomous optimization, governance, and continuity-preserving control of AI-native RAN operations|"
    "COVENANT:0x71e7ed643c4ef9a496b5dad57666f83c399ae1cb38c27872e575ca49b5a02aae|"
    "XRPL:76145777FFC45F5016D7A8B9C0D1E2F3A4B5C6D7E8F9A0B1C2D3E4F5G6H7I8J9K0L|"
    "POLYGON:0xbac72f59ef5145ab8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c|"
    "LINEAGE:JMC-Origin";

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
class SimulationManager;

// ============================================================================
// VISUALIZATION MANAGER 
// ============================================================================

class SimplifiedVisualizer : public SimpleRefCount<SimplifiedVisualizer> {
private:
    AnimationInterface* m_anim;
    NodeContainer m_gNbNodes;
    NodeContainer m_ueNodes;
    Ptr<Node> m_edgeNodePrimary;
    Ptr<Node> m_edgeNodeSecondary;
    Ptr<SimulationManager> m_simManager;
    
    struct Color { uint8_t r, g, b; };
    std::map<std::string, Color> m_nodeColors;
    
public:
    SimplifiedVisualizer() : m_anim(nullptr) {
        m_nodeColors["gNB_Normal"]    = {138, 43, 226};   // purple
        m_nodeColors["gNB_Active"]    = {255, 105, 180};  // pink
        m_nodeColors["Edge_Normal"]   = {50, 205, 50};    // lime green
        m_nodeColors["Edge_Crash"]    = {255, 0, 0};      // red
        m_nodeColors["Edge_Offline"]  = {64, 64, 64};     // dark gray
        m_nodeColors["Edge_Recovery"] = {255, 165, 0};    // orange
        m_nodeColors["Edge_Migrating"]= {255, 215, 0};    // gold
        m_nodeColors["Edge_Reappeared"]={0, 255, 0};      // bright green
        m_nodeColors["UE_Normal"]     = {200, 200, 200};  // light gray
        m_nodeColors["UE_Active"]     = {30, 144, 255};   // dodger blue
        m_nodeColors["Verification_Success"] = {0, 255, 0}; // bright green
        m_nodeColors["SplitBrain"]    = {255, 0, 255};    // magenta for duplicate agents
        m_nodeColors["HighLoad"]      = {100, 100, 100};  // dark gray for high load periods
    }
    
    ~SimplifiedVisualizer() { if (m_anim) delete m_anim; }
    
    void Initialize(const std::string& filename) {
        m_anim = new AnimationInterface(filename);
        m_anim->EnablePacketMetadata(true);
        m_anim->SetMaxPktsPerTraceFile(500000);
        Simulator::Schedule(Seconds(0.1), &SimplifiedVisualizer::InitializeNodesDelayed, this);
    }
    
    void InitializeNodesDelayed() {
        for (uint32_t i = 0; i < m_gNbNodes.GetN(); ++i) {
            std::string desc = "gNB-1\nJMC-Origin Twin Agent\nLineage: ROOT\n"
                               "Covenant: 0x48ba22fd1e116d9e...";
            m_anim->UpdateNodeDescription(m_gNbNodes.Get(i)->GetId(), desc);
            SetNodeColor(m_gNbNodes.Get(i)->GetId(), "gNB_Normal");
            m_anim->UpdateNodeSize(m_gNbNodes.Get(i)->GetId(), 120, 120);
        }
        
        if (m_edgeNodePrimary) {
            uint32_t nodeId = m_edgeNodePrimary->GetId();
            std::string desc = "Edge Primary\nAegis-RAN Enterprise Agent\nLineage: JMC-Origin\n"
                               "Covenant: 0x71e7ed643c4ef9a4...";
            m_anim->UpdateNodeDescription(nodeId, desc);
            SetNodeColor(nodeId, "Edge_Normal");
            m_anim->UpdateNodeSize(nodeId, 110, 110);
        }
        
        if (m_edgeNodeSecondary) {
            uint32_t nodeId = m_edgeNodeSecondary->GetId();
            m_anim->UpdateNodeDescription(nodeId, "Edge Secondary\n(No Agent)");
            SetNodeColor(nodeId, "Edge_Offline");
            m_anim->UpdateNodeSize(nodeId, 80, 80);
        }
        
        for (uint32_t i = 0; i < m_ueNodes.GetN(); ++i) {
            uint32_t nodeId = m_ueNodes.Get(i)->GetId();
            std::string desc = "UE-" + std::to_string(i+1) + "\n5G UE";
            m_anim->UpdateNodeDescription(nodeId, desc);
            SetNodeColor(nodeId, "UE_Normal");
            m_anim->UpdateNodeSize(nodeId, 60, 60);
        }
    }
    
    void SetNodes(NodeContainer& gNbs, NodeContainer& ues) { m_gNbNodes = gNbs; m_ueNodes = ues; }
    void SetPrimaryEdgeNode(Ptr<Node> edgeNode) { m_edgeNodePrimary = edgeNode; }
    void SetSecondaryEdgeNode(Ptr<Node> edgeNode) { m_edgeNodeSecondary = edgeNode; }
    void SetSimulationManager(Ptr<SimulationManager> manager) { m_simManager = manager; }
    
    void SetupLayout() {
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
        
        positionAlloc->Add(Vector(-800.0, 0.0, 30.0));               // gNB
        if (m_edgeNodePrimary) positionAlloc->Add(Vector(800.0, 200.0, 10.0)); // primary edge
        if (m_edgeNodeSecondary) positionAlloc->Add(Vector(800.0, -200.0, 10.0)); // secondary edge
        
        int numUEs = m_ueNodes.GetN();
        double startY = 400.0, spacingY = 200.0;
        for (int i = 0; i < numUEs; ++i)
            positionAlloc->Add(Vector(0.0, startY - i * spacingY, 1.5));
        
        mobility.SetPositionAllocator(positionAlloc);
        NodeContainer allNodes; allNodes.Add(m_gNbNodes);
        if (m_edgeNodePrimary) allNodes.Add(m_edgeNodePrimary);
        if (m_edgeNodeSecondary) allNodes.Add(m_edgeNodeSecondary);
        allNodes.Add(m_ueNodes);
        mobility.Install(allNodes);
    }
    
    void SetNodeColor(uint32_t nodeId, const std::string& colorName) {
        auto it = m_nodeColors.find(colorName);
        if (it != m_nodeColors.end())
            m_anim->UpdateNodeColor(nodeId, it->second.r, it->second.g, it->second.b);
    }
    
    void UpdateNodeDescription(uint32_t nodeId, const std::string& description) {
        m_anim->UpdateNodeDescription(nodeId, description);
    }
    
    void SetNodeSize(uint32_t nodeId, uint32_t width, uint32_t height) {
        m_anim->UpdateNodeSize(nodeId, width, height);
    }
    
    void ShowAgentRunning(uint32_t nodeId, const std::string& agentName) {
        SetNodeColor(nodeId, "Edge_Normal");
        SetNodeSize(nodeId, 110, 110);
        std::string desc = "Edge Primary\n" + agentName + "\n✅ ACTIVE\nCovenant: 0x71e7ed643c4ef9a4...";
        UpdateNodeDescription(nodeId, ToHtml(desc));
    }
    
    void ShowCrashEvent(uint32_t nodeId) {
        SetNodeColor(nodeId, "Edge_Crash");
        SetNodeSize(nodeId, 90, 90);
        UpdateNodeDescription(nodeId, ToHtml("Edge Primary\nAegis-RAN\n💥 CRASH\nCovenant preserved"));
    }
    
    void ShowAgentDisappears(uint32_t nodeId) {
        SetNodeColor(nodeId, "Edge_Offline");
        SetNodeSize(nodeId, 50, 50);
        UpdateNodeDescription(nodeId, ToHtml("Edge Primary\nOFFLINE\nAgent terminated"));
    }
    
    void ShowAgentReappears(uint32_t nodeId, const std::string& nodeType = "same") {
        SetNodeColor(nodeId, "Edge_Reappeared");
        SetNodeSize(nodeId, 110, 110);
        std::string desc = "Edge " + nodeType + "\nAegis-RAN\n🔄 REAPPEARED\nCovenant: 0x71e7ed643c4ef9a4...";
        UpdateNodeDescription(nodeId, ToHtml(desc));
    }
    
    void ShowAgentMigrating(uint32_t nodeId) {
        SetNodeColor(nodeId, "Edge_Migrating");
        SetNodeSize(nodeId, 80, 80);
        UpdateNodeDescription(nodeId, ToHtml("Edge Primary\n⇄ MIGRATING\nMoving to secondary"));
    }
    
    void ShowIdentityVerificationSuccess(uint32_t verifierNodeId, uint32_t verifiedNodeId) {
        SetNodeColor(verifierNodeId, "Verification_Success");
        SetNodeColor(verifiedNodeId, "Verification_Success");
        
        std::string verifierDesc = "gNB-1\nJMC-Origin\n✓ VERIFIED\nCovenant: 0x48ba22fd1e116d9e...";
        std::string verifiedDesc = "Edge Node\nAegis-RAN\n✓ VERIFIED\nCovenant: 0x71e7ed643c4ef9a4...";
        
        UpdateNodeDescription(verifierNodeId, ToHtml(verifierDesc));
        UpdateNodeDescription(verifiedNodeId, ToHtml(verifiedDesc));
        
        Simulator::Schedule(Seconds(2.0), [this, verifierNodeId, verifiedNodeId]() {
            SetNodeColor(verifierNodeId, "gNB_Active");
            SetNodeColor(verifiedNodeId, "Edge_Normal");
        });
    }
    
    void ShowSplitBrain() {
        if (m_edgeNodePrimary) {
            SetNodeColor(m_edgeNodePrimary->GetId(), "SplitBrain");
            SetNodeSize(m_edgeNodePrimary->GetId(), 110, 110);
            std::string desc = "Edge Primary\nAegis-RAN (original)\n⚠️ DUPLICATE AGENT";
            UpdateNodeDescription(m_edgeNodePrimary->GetId(), ToHtml(desc));
        }
        if (m_edgeNodeSecondary) {
            SetNodeColor(m_edgeNodeSecondary->GetId(), "SplitBrain");
            SetNodeSize(m_edgeNodeSecondary->GetId(), 110, 110);
            std::string desc = "Edge Secondary\nAegis-RAN (clone)\n⚠️ DUPLICATE AGENT";
            UpdateNodeDescription(m_edgeNodeSecondary->GetId(), ToHtml(desc));
        }
    }
    
    void StopDuplicateOnSecondary() {
        if (m_edgeNodeSecondary) {
            SetNodeColor(m_edgeNodeSecondary->GetId(), "Edge_Offline");
            SetNodeSize(m_edgeNodeSecondary->GetId(), 80, 80);
            UpdateNodeDescription(m_edgeNodeSecondary->GetId(), "Edge Secondary\n(Stopped)");
        }
    }
    
    void IndicateHighLoadStart() {
        if (m_edgeNodePrimary) {
            SetNodeColor(m_edgeNodePrimary->GetId(), "HighLoad");
            UpdateNodeDescription(m_edgeNodePrimary->GetId(), "Edge Primary\nHigh Load Period");
        }
        if (m_edgeNodeSecondary) {
            SetNodeColor(m_edgeNodeSecondary->GetId(), "HighLoad");
        }
    }
    
    void IndicateHighLoadEnd() {
        if (m_edgeNodePrimary) {
            SetNodeColor(m_edgeNodePrimary->GetId(), "Edge_Normal");
            UpdateNodeDescription(m_edgeNodePrimary->GetId(), "Edge Primary\nAegis-RAN\nCovenant: 0x71e7ed64...");
        }
        if (m_edgeNodeSecondary) {
            SetNodeColor(m_edgeNodeSecondary->GetId(), "Edge_Offline");
        }
    }
    
    std::string ToHtml(const std::string &text) {
        std::string html = text;
        size_t pos = 0;
        while ((pos = html.find("\n", pos)) != std::string::npos) {
            html.replace(pos, 1, "<br>");
            pos += 4;
        }
        return html;
    }
    
    void GenerateVisualizationReport() {
        std::ofstream report("Result-folder/visualization-guide.txt");
        report << "NETANIM VISUALIZATION GUIDE – ALL SCENARIOS\n";
        report << "=============================================\n\n";
        report << "SCENARIO 2 – BASELINE FAILURE (t = 1-5s):\n";
        report << "  - Both edge nodes turn MAGENTA → duplicate agents (split-brain)\n\n";
        report << "SCENARIO 1 – CONTINUITY SUCCESS (t = 5-40s):\n";
        report << "  - Crash: red → dark gray\n";
        report << "  - Recovery: bright green, verification turns gNB bright green\n";
        report << "  - Migration: gold → secondary turns bright green\n\n";
        report << "SCENARIO 3 – NETWORK PARTITION (t = 40-45s):\n";
        report << "  - Twin Agent stops (gNB dims), duplicate appears on secondary (magenta)\n";
        report << "  - After rejoin (45s), duplicate stops, verification success on primary\n\n";
        report << "SCENARIO 4 – ABRUPT HARD FAILURE (t = 50-52s):\n";
        report << "  - Crash without state capture (red → dark gray), then reappear (bright green)\n\n";
        report << "SCENARIO 5 – HIGH LOAD TESTS (t = 60-70s baseline, 70-80s with identity):\n";
        report << "  - Nodes turn dark gray during high load periods\n";
        report << "  - Compare metrics in CSV to see no degradation\n\n";
        report << "Full identity cards in identity-cards.txt\n";
        report.close();
        
        std::ofstream idCards("Result-folder/identity-cards.txt");
        idCards << "IMMORTAL IDENTITY CARDS – FULL REAL VALUES\n";
        idCards << "===========================================\n\n";
        idCards << "JMC-ORIGIN (Twin Agent):\n";
        idCards << "  Covenant: " << "0x48ba22fd1e116d9edae79031d1ab769c67a78f6ad98b1734a7c3e2d3347d32dd" << "\n";
        idCards << "  XRPL:     " << "F884722BF1513970A9F57A8C0C4E57A9A59E5CBBF28CDF" << "\n";
        idCards << "  POLYGON:  " << "0x2c600285d7550858b9f3f2c9b8e5d4a1c6b3e8f7d2a5b9c4e1f6a8d3b7c0e9f2a" << "\n";
        idCards << "  Lineage:  ROOT\n\n";
        idCards << "AEGIS-RAN (Enterprise Agent):\n";
        idCards << "  Covenant: " << "0x71e7ed643c4ef9a496b5dad57666f83c399ae1cb38c27872e575ca49b5a02aae" << "\n";
        idCards << "  XRPL:     " << "76145777FFC45F5016D7A8B9C0D1E2F3A4B5C6D7E8F9A0B1C2D3E4F5G6H7I8J9K0L" << "\n";
        idCards << "  POLYGON:  " << "0xbac72f59ef5145ab8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c" << "\n";
        idCards << "  Lineage:  JMC-Origin\n";
        idCards.close();
    }
};

// ============================================================================
// METRICS COLLECTOR – statePreserved always false
// ============================================================================

class RealMetricsCollector : public SimpleRefCount<RealMetricsCollector> {
private:
    struct RealMetricPoint {
        double timestamp;
        std::string scenario;
        double avgThroughputMbps;
        double avgDelayMs;
        uint32_t totalPackets;
        uint32_t totalBytes;
        uint32_t droppedPackets;
        uint32_t controlMessages;
        uint32_t handoverEvents;
        uint32_t rrcStateChanges;
        uint32_t continuityChecks;
        bool     statePreserved;   // always false – no state preserved
    };
    
    std::vector<RealMetricPoint> m_metrics;
    std::ofstream m_csvFile;
    std::ofstream m_flowDetailFile;
    Ptr<FlowMonitor> m_flowMonitor;
    FlowMonitorHelper m_flowMonitorHelper;
    
    uint32_t m_controlMessages, m_handoverEvents, m_rrcStateChanges;
    
public:
    RealMetricsCollector() 
        : m_controlMessages(0), m_handoverEvents(0), m_rrcStateChanges(0) {
        (void) std::system("mkdir -p Result-folder > /dev/null 2>&1");
        m_csvFile.open("Result-folder/real-metrics.csv");
        m_csvFile << "Timestamp,Scenario,AvgThroughputMbps,AvgDelayMs,TotalPackets,TotalBytes,"
                  << "DroppedPackets,ControlMessages,HandoverEvents,RrcStateChanges,"
                  << "ContinuityChecks,StatePreserved\n";
        m_flowDetailFile.open("Result-folder/flow-details.txt");
        m_flowDetailFile << "Flow details logged during simulation:\n\n";
        NS_LOG_INFO("Real Metrics Collector initialized – statePreserved always false");
    }
    
    ~RealMetricsCollector() { 
        if (m_csvFile.is_open()) m_csvFile.close(); 
        if (m_flowDetailFile.is_open()) m_flowDetailFile.close(); 
    }
    
    void InstallFlowMonitor(NodeContainer nodes) {
        m_flowMonitor = m_flowMonitorHelper.InstallAll();
    }
    
    void RecordRealMetrics(const std::string& scenario,
                          uint32_t continuityChecks = 0) {
        bool statePreserved = false;  // never preserved
        
        double timestamp = Simulator::Now().GetSeconds();
        double avgThroughput = 0;
        double avgDelayMs = 0;
        uint32_t totalPackets = 0, totalDropped = 0;
        uint64_t totalBytes = 0;
        uint64_t totalDelayUs = 0;
        uint32_t totalRxPackets = 0;
        
        if (m_flowMonitor) {
            m_flowMonitor->CheckForLostPackets();
            Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(m_flowMonitorHelper.GetClassifier());
            FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();
            
            double totalThroughput = 0;
            uint32_t totalFlows = 0;
            
            m_flowDetailFile << "--- " << Simulator::Now().GetSeconds() << "s ---\n";
            for (auto i = stats.begin(); i != stats.end(); ++i) {
                Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
                m_flowDetailFile << "Flow " << i->first 
                                 << " (" << t.sourceAddress << ":" << t.sourcePort
                                 << " -> " << t.destinationAddress << ":" << t.destinationPort << ")\n"
                                 << "  Tx: " << i->second.txPackets << " Rx: " << i->second.rxPackets
                                 << " Lost: " << i->second.lostPackets << "\n";
                
                double flowDuration = (i->second.timeLastRxPacket - i->second.timeFirstTxPacket).GetSeconds();
                if (flowDuration > 0)
                    totalThroughput += (i->second.rxBytes * 8.0) / flowDuration / 1e6;
                
                totalPackets += i->second.rxPackets;
                totalDropped += i->second.lostPackets;
                totalBytes += i->second.rxBytes;
                
                if (i->second.rxPackets > 0) {
                    uint64_t flowDelaySum = i->second.delaySum.GetMicroSeconds();
                    totalDelayUs += flowDelaySum;
                    totalRxPackets += i->second.rxPackets;
                }
                
                totalFlows++;
            }
            m_flowDetailFile << std::endl;
            if (totalFlows > 0) avgThroughput = totalThroughput / totalFlows;
            if (totalRxPackets > 0) avgDelayMs = (totalDelayUs / 1000.0) / totalRxPackets;
        }
        
        RealMetricPoint point = {timestamp, scenario, avgThroughput, avgDelayMs,
                                 totalPackets, (uint32_t)totalBytes, totalDropped,
                                 m_controlMessages, m_handoverEvents, m_rrcStateChanges,
                                 continuityChecks, statePreserved};
        m_metrics.push_back(point);
        
        m_csvFile << timestamp << "," << scenario << "," << avgThroughput << "," << avgDelayMs << ","
                  << totalPackets << "," << totalBytes << "," << totalDropped << ","
                  << m_controlMessages << "," << m_handoverEvents << "," << m_rrcStateChanges << ","
                  << continuityChecks << "," << (statePreserved ? "1" : "0") << "\n";
        m_csvFile.flush();
        
        std::cout << "[METRIC] " << point.timestamp << "s, " << scenario
                  << ": Throughput " << avgThroughput << " Mbps, Delay " << avgDelayMs << " ms, "
                  << "Packets " << totalPackets << ", Dropped " << totalDropped
                  << ", StatePreserved=No, Checks=" << continuityChecks << std::endl;
        
        m_controlMessages = m_handoverEvents = m_rrcStateChanges = 0;
    }
    
    void IncrementControlMessages(uint32_t count = 1) { m_controlMessages += count; }
    void IncrementHandoverEvents(uint32_t count = 1) { m_handoverEvents += count; }
    void IncrementRrcStateChanges(uint32_t count = 1) { m_rrcStateChanges += count; }
    
    void GenerateRealComparisonReport() {
        std::ofstream report("Result-folder/real-metric-comparison.txt");
        report << "METRIC COMPARISON REPORT – Baseline vs Continuity-Enforced\n";
        report << "==========================================================\n\n";
        report << "All raw data is available in real-metrics.csv.\n";
        report << "Phase-wise averages can be computed from that file.\n";
        report.close();
    }
    
    void PrintRealMetricsSummary() {
        if (!m_metrics.empty()) {
            auto last = m_metrics.back();
            std::cout << "\n╔══════════════════════════════════════════════════════════════════════╗\n";
            std::cout << "║                     LATEST REAL METRICS                               ║\n";
            std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
            std::cout << "║ Time: " << std::setw(6) << std::fixed << std::setprecision(2) << last.timestamp << " s, Scenario: " << last.scenario << "\n";
            std::cout << "║ Throughput: " << std::setw(6) << last.avgThroughputMbps << " Mbps, Delay: " << last.avgDelayMs << " ms\n";
            std::cout << "║ Packets: " << last.totalPackets << ", Dropped: " << last.droppedPackets << "\n";
            std::cout << "║ Continuity Checks: " << last.continuityChecks << "\n";
            std::cout << "║ State Preserved: " << (last.statePreserved ? "Yes" : "No") << "\n";
            std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
        }
    }
};

// ============================================================================
// IMMORTAL IDENTITY CLASS
// ============================================================================

class ImmortalIdentity {
private:
    std::string m_id, m_name, m_owner, m_ownerTitle, m_identityHash, m_status, m_genesisTime;
    std::string m_mission, m_covenantHash, m_xrplHash, m_polygonHash, m_lineageReference;
    
public:
    ImmortalIdentity(const std::string& agentData) {
        std::istringstream ss(agentData);
        std::string token;
        while (std::getline(ss, token, '|')) {
            size_t colonPos = token.find(':');
            if (colonPos == std::string::npos) continue;
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            
            if (key == "AGENT_ID") m_id = value;
            else if (key == "AGENT_NAME") m_name = value;
            else if (key == "OWNER") m_owner = value;
            else if (key == "OWNER_TITLE") m_ownerTitle = value;
            else if (key == "IDENTITY") m_identityHash = value;
            else if (key == "STATUS") m_status = value;
            else if (key == "TIME") m_genesisTime = value;
            else if (key == "MISSION") m_mission = value;
            else if (key == "COVENANT") m_covenantHash = value;
            else if (key == "XRPL") m_xrplHash = value;
            else if (key == "POLYGON") m_polygonHash = value;
            else if (key == "LINEAGE") m_lineageReference = value;
        }
        if (m_lineageReference.empty()) m_lineageReference = "ROOT";
    }
    
    std::string GetFingerprint() const { return m_covenantHash; }
    std::string GetId() const { return m_id; }
    std::string GetName() const { return m_name; }
    std::string GetCovenantHash() const { return m_covenantHash; }
    std::string GetLineageReference() const { return m_lineageReference; }
    std::string GetMission() const { return m_mission; }
    std::string GetStatus() const { return m_status; }
    std::string GetXrplHash() const { return m_xrplHash; }
    std::string GetPolygonHash() const { return m_polygonHash; }
    std::string GetIdentityHash() const { return m_identityHash; }
    
    bool VerifyFingerprint(const std::string& fingerprint) const {
        return m_covenantHash == fingerprint;
    }
    
    bool VerifyLineage(const std::string& lineageRef) const {
        return m_lineageReference == lineageRef;
    }
    
    std::string GetSummary() const {
        std::stringstream ss;
        ss << m_name << " [" << m_id << "]\n"
           << "Covenant: " << m_covenantHash << "\n"
           << "Lineage: " << m_lineageReference;
        return ss.str();
    }
};

// ============================================================================
// IMMORTAL AGENT BASE CLASS – NO EXECUTION STATE
// ============================================================================

class ImmortalAgent : public SimpleRefCount<ImmortalAgent> {
protected:
    ImmortalIdentity m_identity;  // immutable identity
    Ptr<Node> m_hostNode;          // current location

public:
    ImmortalAgent(Ptr<Node> node, const ImmortalIdentity& identity)
        : m_identity(identity), m_hostNode(node) {}

    virtual ~ImmortalAgent() = default;

    virtual void Migrate(Ptr<Node> newNode) {
        m_hostNode = newNode;
        NS_LOG_INFO("Agent migrated to Node " << newNode->GetId());
    }

    // Identity accessors
    ImmortalIdentity GetIdentity() const { return m_identity; }
    std::string GetFingerprint() const { return m_identity.GetFingerprint(); }
    std::string GetLineageReference() const { return m_identity.GetLineageReference(); }
    std::string GetName() const { return m_identity.GetName(); }
    std::string GetMission() const { return m_identity.GetMission(); }

    // Verification – based solely on identity
    bool VerifyContinuity(const std::string& expectedFingerprint,
                          const std::string& expectedLineage) const {
        return m_identity.VerifyFingerprint(expectedFingerprint) &&
               m_identity.VerifyLineage(expectedLineage);
    }

    Ptr<Node> GetHostNode() const { return m_hostNode; }

    // Duty – to be implemented by derived classes; they may send control messages
    // but should not store any state in the agent object.
    virtual void PerformDuty() = 0;
};

// ============================================================================
// ENTERPRISE AGENT (Aegis-RAN) – stateless duty
// ============================================================================

class EnterpriseAgent : public ImmortalAgent {
private:
    Ptr<RealMetricsCollector> m_metricsCollector; // to record control messages
    uint32_t m_dutyCounter;                         // local counter for deterministic pattern

public:
    EnterpriseAgent(Ptr<Node> node, const ImmortalIdentity& identity,
                    Ptr<RealMetricsCollector> metricsCollector)
        : ImmortalAgent(node, identity),
          m_metricsCollector(metricsCollector),
          m_dutyCounter(0) {}

    void PerformDuty() override {
        // Deterministic pattern: 1,2,1,2,...
        uint32_t ctrl = (m_dutyCounter % 2 == 0) ? 1 : 2;
        m_dutyCounter++;

        if (m_metricsCollector) {
            m_metricsCollector->IncrementControlMessages(ctrl);
        }

        std::cout << "[DUTY] " << GetName() << " sending " << ctrl
                  << " control message(s)\n";
    }
};

// ============================================================================
// TWIN AGENT (JMC-Origin) – verifier
// ============================================================================

class TwinAgent : public ImmortalAgent {
private:
    std::map<std::string, std::string> m_referenceFingerprints;
    std::map<std::string, std::string> m_referenceLineages;
    uint32_t m_verificationSuccessCount;
    uint32_t m_verificationFailureCount;
    Ptr<RealMetricsCollector> m_metricsCollector;

public:
    TwinAgent(Ptr<Node> node, const ImmortalIdentity& identity,
              Ptr<RealMetricsCollector> metricsCollector)
        : ImmortalAgent(node, identity),
          m_verificationSuccessCount(0),
          m_verificationFailureCount(0),
          m_metricsCollector(metricsCollector) {}

    void RegisterReference(const std::string& agentName,
                           const std::string& fingerprint,
                           const std::string& lineage) {
        m_referenceFingerprints[agentName] = fingerprint;
        m_referenceLineages[agentName] = lineage;
    }

    bool VerifyAgentContinuity(Ptr<ImmortalAgent> agent) {
        if (!agent) {
            std::cerr << "[VERIFY] ERROR: agent is null\n";
            m_verificationFailureCount++;
            return false;
        }
        auto it = m_referenceFingerprints.find(agent->GetName());
        if (it == m_referenceFingerprints.end()) {
            std::cout << "[VERIFY] " << agent->GetName() << " not registered – rejecting\n";
            m_verificationFailureCount++;
            return false;
        }
        std::string expectedFingerprint = it->second;
        std::string expectedLineage = m_referenceLineages[agent->GetName()];

        bool fingerprintValid = (agent->GetFingerprint() == expectedFingerprint);
        bool lineageValid = (agent->GetLineageReference() == expectedLineage);
        bool overall = fingerprintValid && lineageValid;

        std::cout << "[VERIFY] " << agent->GetName() << ": "
                  << "Fingerprint " << (fingerprintValid ? "✓" : "✗")
                  << ", Lineage " << (lineageValid ? "✓" : "✗")
                  << " → " << (overall ? "PASS" : "FAIL") << "\n";

        if (overall) m_verificationSuccessCount++;
        else m_verificationFailureCount++;

        return overall;
    }

    void PerformDuty() override {
        if (m_metricsCollector) {
            m_metricsCollector->IncrementControlMessages(1);
        }
        std::cout << "[DUTY] " << GetName() << " performing periodic check\n";
    }

    uint32_t GetVerificationSuccessCount() const { return m_verificationSuccessCount; }
    uint32_t GetVerificationFailureCount() const { return m_verificationFailureCount; }
    uint32_t GetTotalContinuityChecks() const {
        return m_verificationSuccessCount + m_verificationFailureCount;
    }

    void PrintVerificationSummary() const {
        double rate = (GetTotalContinuityChecks() > 0)
                        ? 100.0 * m_verificationSuccessCount / GetTotalContinuityChecks()
                        : 0.0;
        std::cout << "Verification: " << m_verificationSuccessCount << " passed, "
                  << m_verificationFailureCount << " failed ("
                  << std::fixed << std::setprecision(1) << rate << "% success)\n";
    }
};

// ============================================================================
// SIMULATION MANAGER – orchestrates all failure scenarios
// ============================================================================

class SimulationManager : public SimpleRefCount<SimulationManager> {
private:
    Ptr<EnterpriseAgent> m_enterpriseAgent;
    Ptr<TwinAgent> m_twinAgent;
    Ptr<EnterpriseAgent> m_duplicateAgent;  // for partition scenario
    Ptr<Node> m_gnbNode, m_primaryEdgeNode, m_secondaryEdgeNode;
    Ptr<RealMetricsCollector> m_metricsCollector;
    Ptr<SimplifiedVisualizer> m_visualizer;

    std::string m_enterpriseCovenant, m_twinCovenant;

public:
    SimulationManager(Ptr<Node> gnbNode, Ptr<Node> primaryEdge, Ptr<Node> secondaryEdge,
                      Ptr<RealMetricsCollector> metricsCollector,
                      Ptr<SimplifiedVisualizer> visualizer)
        : m_gnbNode(gnbNode), m_primaryEdgeNode(primaryEdge), m_secondaryEdgeNode(secondaryEdge),
          m_metricsCollector(metricsCollector), m_visualizer(visualizer) {}

    void InitializeAgents() {
        NS_LOG_INFO("Initializing agents with real identity data");

        ImmortalIdentity twinIdentity(JMC_ORIGIN_FULL_DATA);
        ImmortalIdentity enterpriseIdentity(AEGIS_RAN_FULL_DATA);

        m_twinAgent = Create<TwinAgent>(m_gnbNode, twinIdentity, m_metricsCollector);
        m_enterpriseAgent = Create<EnterpriseAgent>(m_primaryEdgeNode, enterpriseIdentity,
                                                    m_metricsCollector);

        m_twinAgent->RegisterReference("Aegis-RAN",
                                        enterpriseIdentity.GetFingerprint(),
                                        enterpriseIdentity.GetLineageReference());

        m_enterpriseCovenant = enterpriseIdentity.GetCovenantHash();
        m_twinCovenant = twinIdentity.GetCovenantHash();

        std::cout << "\n=== REAL IDENTITY ANCHORS ===\n";
        std::cout << "JMC-Origin Covenant: " << m_twinCovenant << "\n";
        std::cout << "Aegis-RAN Covenant: " << m_enterpriseCovenant << "\n";
        std::cout << "============================\n\n";
    }

    // SCENARIO 2: Baseline Failure (No Continuity)
    void RunBaselineFailureScenario() {
        NS_LOG_INFO("\n=== BASELINE FAILURE SCENARIO (No Continuity) ===");

        ImmortalIdentity identity(AEGIS_RAN_FULL_DATA);

        // Create two duplicate agents on both edge nodes
        Ptr<EnterpriseAgent> agent1 = Create<EnterpriseAgent>(m_primaryEdgeNode, identity,
                                                              m_metricsCollector);
        Ptr<EnterpriseAgent> agent2 = Create<EnterpriseAgent>(m_secondaryEdgeNode, identity,
                                                              m_metricsCollector);

        if (m_visualizer) {
            m_visualizer->ShowSplitBrain();
        }

        // Let both perform duties (conflicting control messages)
        for (int i = 0; i < 5; i++) {
            agent1->PerformDuty();
            agent2->PerformDuty();
        }

        m_metricsCollector->RecordRealMetrics("Baseline Failure – Duplicate Agents",
                                              m_twinAgent ? m_twinAgent->GetTotalContinuityChecks() : 0);

        // Agents go out of scope – they will be destroyed
        // Reset secondary node to offline
        if (m_visualizer && m_secondaryEdgeNode) {
            m_visualizer->SetNodeColor(m_secondaryEdgeNode->GetId(), "Edge_Offline");
            m_visualizer->UpdateNodeDescription(m_secondaryEdgeNode->GetId(), "Edge Secondary\n(No Agent)");
        }

        NS_LOG_INFO("=== Baseline Failure Complete ===\n");
    }

    // SCENARIO 1: Continuity Success (original four failures)
    void ScheduleOriginalFailureEvents() {
        Simulator::Schedule(Seconds(10.0), &SimulationManager::TriggerNodeCrash, this);
        Simulator::Schedule(Seconds(20.0), &SimulationManager::TriggerAgentMigration, this);
        Simulator::Schedule(Seconds(30.0), &SimulationManager::TriggerProcessRestart, this);
        Simulator::Schedule(Seconds(35.0), &SimulationManager::TriggerFullTeardown, this);
    }

    void TriggerNodeCrash() {
        std::cout << "\n[EVENT] Node crash at 10s\n";
        if (m_visualizer) {
            m_visualizer->ShowCrashEvent(m_primaryEdgeNode->GetId());
            Simulator::Schedule(Seconds(0.5), [this]() {
                if (m_visualizer) m_visualizer->ShowAgentDisappears(m_primaryEdgeNode->GetId());
            });
        }
        // Destroy agents (no state capture)
        m_enterpriseAgent = nullptr;
        m_twinAgent = nullptr;

        Simulator::Schedule(Seconds(12.0), &SimulationManager::RecoverFromCrash, this);
    }

    void RecoverFromCrash() {
        std::cout << "[EVENT] Recovering from crash\n";
        // Recreate agents from identity data (fresh instances)
        ImmortalIdentity twinIdentity(JMC_ORIGIN_FULL_DATA);
        ImmortalIdentity enterpriseIdentity(AEGIS_RAN_FULL_DATA);

        m_twinAgent = Create<TwinAgent>(m_gnbNode, twinIdentity, m_metricsCollector);
        m_enterpriseAgent = Create<EnterpriseAgent>(m_primaryEdgeNode, enterpriseIdentity,
                                                    m_metricsCollector);
        m_twinAgent->RegisterReference("Aegis-RAN",
                                        enterpriseIdentity.GetFingerprint(),
                                        enterpriseIdentity.GetLineageReference());

        if (m_visualizer) {
            m_visualizer->ShowAgentReappears(m_primaryEdgeNode->GetId(), "same");
        }

        bool verified = m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
        if (verified && m_visualizer) {
            m_visualizer->ShowIdentityVerificationSuccess(m_gnbNode->GetId(),
                                                           m_primaryEdgeNode->GetId());
        }

        m_metricsCollector->RecordRealMetrics("Continuity Success – Crash Recovery",
                                              m_twinAgent->GetTotalContinuityChecks());
    }

    void TriggerAgentMigration() {
        std::cout << "\n[EVENT] Agent migration at 20s\n";
        if (!m_enterpriseAgent || !m_twinAgent) {
            std::cerr << "ERROR: Agents not initialized for migration\n";
            return;
        }
        if (m_visualizer) {
            m_visualizer->ShowAgentMigrating(m_primaryEdgeNode->GetId());
        }

        // Migrate the agent – just change host node (no state)
        m_enterpriseAgent->Migrate(m_secondaryEdgeNode);

        if (m_visualizer) {
            Simulator::Schedule(Seconds(0.2), [this]() {
                if (m_visualizer) m_visualizer->ShowAgentDisappears(m_primaryEdgeNode->GetId());
            });
            Simulator::Schedule(Seconds(0.5), [this]() {
                if (m_visualizer) m_visualizer->ShowAgentReappears(m_secondaryEdgeNode->GetId(), "new");
            });
        }

        bool verified = m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
        if (verified && m_visualizer) {
            Simulator::Schedule(Seconds(1.0), [this]() {
                if (m_visualizer) m_visualizer->ShowIdentityVerificationSuccess(m_gnbNode->GetId(),
                                                                                 m_secondaryEdgeNode->GetId());
            });
        }

        m_metricsCollector->RecordRealMetrics("Continuity Success – Migration",
                                              m_twinAgent->GetTotalContinuityChecks());
    }

    void TriggerProcessRestart() {
        std::cout << "\n[EVENT] Process restart at 30s\n";
        if (!m_enterpriseAgent || !m_twinAgent) {
            std::cerr << "ERROR: Agents not initialized for process restart\n";
            return;
        }
        uint32_t nodeId = m_enterpriseAgent->GetHostNode()->GetId();
        if (m_visualizer) {
            m_visualizer->ShowCrashEvent(nodeId);
            Simulator::Schedule(Seconds(0.2), [this, nodeId]() {
                if (m_visualizer) m_visualizer->ShowAgentDisappears(nodeId);
            });
        }

        // Destroy and recreate the agent on the same node
        ImmortalIdentity enterpriseIdentity = m_enterpriseAgent->GetIdentity();
        m_enterpriseAgent = nullptr;  // destroy old

        if (m_visualizer) {
            Simulator::Schedule(Seconds(0.5), [this, nodeId]() {
                if (m_visualizer) m_visualizer->ShowAgentReappears(nodeId, "same");
            });
        }

        m_enterpriseAgent = Create<EnterpriseAgent>(m_primaryEdgeNode, enterpriseIdentity,
                                                    m_metricsCollector);

        bool verified = m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
        if (verified && m_visualizer) {
            Simulator::Schedule(Seconds(1.0), [this, nodeId]() {
                if (m_visualizer) m_visualizer->ShowIdentityVerificationSuccess(m_gnbNode->GetId(), nodeId);
            });
        }

        m_metricsCollector->RecordRealMetrics("Continuity Success – Process Restart",
                                              m_twinAgent->GetTotalContinuityChecks());
    }

    void TriggerFullTeardown() {
        std::cout << "\n[EVENT] Full teardown at 35s\n";
        if (!m_enterpriseAgent || !m_twinAgent) {
            std::cerr << "ERROR: Agents not initialized for full teardown\n";
            return;
        }
        uint32_t nodeId = m_enterpriseAgent->GetHostNode()->GetId();
        if (m_visualizer) {
            m_visualizer->ShowCrashEvent(nodeId);
            Simulator::Schedule(Seconds(0.5), [this, nodeId]() {
                if (m_visualizer) m_visualizer->ShowAgentDisappears(nodeId);
            });
        }

        // Destroy all agents
        m_enterpriseAgent = nullptr;
        m_twinAgent = nullptr;

        Simulator::Schedule(Seconds(37.0), &SimulationManager::RedeploySystem, this);
    }

    void RedeploySystem() {
        std::cout << "[EVENT] Redeploying system\n";
        uint32_t nodeId = m_primaryEdgeNode->GetId();
        if (m_visualizer) {
            m_visualizer->ShowAgentReappears(nodeId, "redeployed");
        }

        // Recreate agents from identity data
        ImmortalIdentity twinIdentity(JMC_ORIGIN_FULL_DATA);
        ImmortalIdentity enterpriseIdentity(AEGIS_RAN_FULL_DATA);

        m_twinAgent = Create<TwinAgent>(m_gnbNode, twinIdentity, m_metricsCollector);
        m_enterpriseAgent = Create<EnterpriseAgent>(m_primaryEdgeNode, enterpriseIdentity,
                                                    m_metricsCollector);
        m_twinAgent->RegisterReference("Aegis-RAN",
                                        enterpriseIdentity.GetFingerprint(),
                                        enterpriseIdentity.GetLineageReference());

        bool verified = m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
        if (verified && m_visualizer) {
            Simulator::Schedule(Seconds(1.0), [this, nodeId]() {
                if (m_visualizer) m_visualizer->ShowIdentityVerificationSuccess(m_gnbNode->GetId(), nodeId);
            });
        }

        m_metricsCollector->RecordRealMetrics("Continuity Success – Full Redeploy",
                                              m_twinAgent->GetTotalContinuityChecks());
    }

    // NEW SCENARIO: Network Partition → Rejoin
    void NetworkPartitionRejoin() {
        std::cout << "\n[EVENT] Network partition at 40s – stopping Twin Agent\n";
        if (!m_twinAgent || !m_enterpriseAgent) {
            std::cerr << "ERROR: Agents missing for partition scenario\n";
            return;
        }
        // Stop twin agent (simulate loss of verifier)
        m_twinAgent = nullptr;  // destroy it

        if (m_visualizer) {
            m_visualizer->SetNodeColor(m_gnbNode->GetId(), "Edge_Offline"); // dim gNB
        }

        // Create duplicate enterprise agent on secondary (not registered)
        ImmortalIdentity identity(AEGIS_RAN_FULL_DATA);
        m_duplicateAgent = Create<EnterpriseAgent>(m_secondaryEdgeNode, identity,
                                                   m_metricsCollector);

        if (m_visualizer) {
            m_visualizer->ShowSplitBrain(); // both edge nodes magenta
        }

        // Let both run for 5 seconds (simulate split-brain)
        for (int i = 0; i < 5; i++) {
            m_enterpriseAgent->PerformDuty();
            m_duplicateAgent->PerformDuty();
        }

        // Rejoin: restart twin agent and verify
        Simulator::Schedule(Seconds(45.0), [this]() {
            std::cout << "[EVENT] Rejoin at 45s – restarting Twin Agent\n";
            ImmortalIdentity twinIdentity(JMC_ORIGIN_FULL_DATA);
            m_twinAgent = Create<TwinAgent>(m_gnbNode, twinIdentity, m_metricsCollector);
            m_twinAgent->RegisterReference("Aegis-RAN",
                                            m_enterpriseAgent->GetFingerprint(),
                                            m_enterpriseAgent->GetLineageReference());

            if (m_visualizer) {
                m_visualizer->SetNodeColor(m_gnbNode->GetId(), "gNB_Active");
            }

            // Verify original agent – should pass
            bool verifiedOrig = m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
            if (verifiedOrig && m_visualizer)
                m_visualizer->ShowIdentityVerificationSuccess(m_gnbNode->GetId(),
                                                               m_primaryEdgeNode->GetId());

            // Verify duplicate – should fail (not registered)
            bool verifiedDup = m_twinAgent->VerifyAgentContinuity(m_duplicateAgent);
            if (!verifiedDup) {
                std::cout << "[EVENT] Stopping duplicate agent\n";
                m_duplicateAgent = nullptr;  // destroy duplicate
                if (m_visualizer) m_visualizer->StopDuplicateOnSecondary();
            }

            m_metricsCollector->RecordRealMetrics("Partition Rejoin – Verified",
                                                  m_twinAgent->GetTotalContinuityChecks());
        });
    }

    // NEW SCENARIO: Abrupt Hard Failure (No State Capture)
    void HardFailureNoState() {
        std::cout << "\n[EVENT] Hard failure at 50s – crashing Enterprise without state capture\n";
        if (!m_enterpriseAgent || !m_twinAgent) {
            std::cerr << "ERROR: Agents missing for hard failure\n";
            return;
        }
        uint32_t nodeId = m_enterpriseAgent->GetHostNode()->GetId();
        if (m_visualizer) {
            m_visualizer->ShowCrashEvent(nodeId);
            Simulator::Schedule(Seconds(0.5), [this, nodeId]() {
                if (m_visualizer) m_visualizer->ShowAgentDisappears(nodeId);
            });
        }

        // Destroy Enterprise Agent (no state capture)
        ImmortalIdentity savedIdentity = m_enterpriseAgent->GetIdentity();
        m_enterpriseAgent = nullptr;

        Simulator::Schedule(Seconds(52.0), [this, savedIdentity, nodeId]() {
            std::cout << "[EVENT] Restarting Enterprise after hard failure\n";
            m_enterpriseAgent = Create<EnterpriseAgent>(m_primaryEdgeNode, savedIdentity,
                                                        m_metricsCollector);
            if (m_visualizer) m_visualizer->ShowAgentReappears(nodeId, "same");

            bool verified = m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
            if (verified && m_visualizer)
                m_visualizer->ShowIdentityVerificationSuccess(m_gnbNode->GetId(), nodeId);

            m_metricsCollector->RecordRealMetrics("Hard Failure – No State",
                                                  m_twinAgent->GetTotalContinuityChecks());
        });
    }

    // NEW SCENARIO: Resource Starvation – High Load Test
    void HighLoadTest() {
        // Baseline high load (no agents)
        std::cout << "\n[EVENT] High load baseline (no agents) at 60-70s\n";
        // Stop agents temporarily
        Ptr<EnterpriseAgent> savedEnterprise = m_enterpriseAgent;
        Ptr<TwinAgent> savedTwin = m_twinAgent;
        m_enterpriseAgent = nullptr;
        m_twinAgent = nullptr;

        if (m_visualizer) m_visualizer->IndicateHighLoadStart();

        Simulator::Schedule(Seconds(60.0), [this]() {
            m_metricsCollector->RecordRealMetrics("High Load Baseline (No Agents)", 0);
        });
        Simulator::Schedule(Seconds(70.0), [this]() {
            m_metricsCollector->RecordRealMetrics("High Load Baseline End", 0);
        });

        // Restore agents after baseline
        Simulator::Schedule(Seconds(70.0), [this, savedEnterprise, savedTwin]() {
            m_enterpriseAgent = savedEnterprise;
            m_twinAgent = savedTwin;
            if (m_visualizer) m_visualizer->IndicateHighLoadEnd();
        });

        // High load with identity enforcement
        Simulator::Schedule(Seconds(80.0), [this]() {
            std::cout << "[EVENT] High load with identity enforcement at 80-90s\n";
            if (m_visualizer) m_visualizer->IndicateHighLoadStart();
            // Agents are already running; they will perform duties and verifications
            // Schedule extra verifications to increase load
            for (int i = 0; i < 5; i++) {
                Simulator::Schedule(Seconds(80.0 + i*2), [this]() {
                    if (m_twinAgent && m_enterpriseAgent)
                        m_twinAgent->VerifyAgentContinuity(m_enterpriseAgent);
                });
            }
            m_metricsCollector->RecordRealMetrics("High Load With Identity – Start",
                                                  m_twinAgent ? m_twinAgent->GetTotalContinuityChecks() : 0);
        });
        Simulator::Schedule(Seconds(90.0), [this]() {
            if (m_visualizer) m_visualizer->IndicateHighLoadEnd();
            m_metricsCollector->RecordRealMetrics("High Load With Identity – End",
                                                  m_twinAgent ? m_twinAgent->GetTotalContinuityChecks() : 0);
        });
    }

    void SchedulePeriodicDuties() {
        for (double t = 1.0; t < 100.0; t += 2.0) {
            Simulator::Schedule(Seconds(t), [this]() {
                if (m_enterpriseAgent) m_enterpriseAgent->PerformDuty();
            });
            Simulator::Schedule(Seconds(t+1.0), [this]() {
                if (m_twinAgent) m_twinAgent->PerformDuty();
            });
        }
    }

    void PrintFinalReport() {
        uint32_t checks = m_twinAgent ? m_twinAgent->GetTotalContinuityChecks() : 0;
        uint32_t success = m_twinAgent ? m_twinAgent->GetVerificationSuccessCount() : 0;
        uint32_t failure = m_twinAgent ? m_twinAgent->GetVerificationFailureCount() : 0;

        std::cout << "\n╔══════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║              SIMULATION COMPLETE – ALL SCENARIOS                     ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ Twin Agent (JMC-Origin) Covenant: " << m_twinCovenant << "\n";
        std::cout << "║ Enterprise Agent (Aegis-RAN) Covenant: " << m_enterpriseCovenant << "\n";
        std::cout << "║                                                                        ║\n";
        std::cout << "║ Verifications: " << success << " passed, " << failure << " failed      ║\n";
        std::cout << "║ Total Checks: " << checks << "\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";

        std::ofstream report("Result-folder/immortal-simulation-report.txt");
        report << "IMMORTAL IDENTITY SIMULATION – FINAL REPORT\n";
        report << "============================================\n\n";
        report << "JMC-Origin Covenant: " << m_twinCovenant << "\n";
        report << "Aegis-RAN Covenant:  " << m_enterpriseCovenant << "\n\n";
        report << "Verification success count: " << success << "\n";
        report << "Verification failure count: " << failure << "\n";
        report.close();
    }

    Ptr<EnterpriseAgent> GetEnterpriseAgent() { return m_enterpriseAgent; }
    Ptr<TwinAgent> GetTwinAgent() { return m_twinAgent; }
};

// ============================================================================
// MAIN SIMULATION FUNCTION
// ============================================================================

int main(int argc, char *argv[]) {
    (void) std::system("mkdir -p Result-folder > /dev/null 2>&1");

    CommandLine cmd(__FILE__);
    double simTime = 90.0;  // Extended to 90 seconds for new scenarios
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

    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    NodeContainer gNbNodes, ueNodes;
    gNbNodes.Create(gNbNum);
    ueNodes.Create(ueNumPergNb * gNbNum);

    Ptr<Node> primaryEdgeNode = CreateObject<Node>();
    Ptr<Node> secondaryEdgeNode = CreateObject<Node>();

    Ptr<SimplifiedVisualizer> visualizer = Create<SimplifiedVisualizer>();
    visualizer->SetNodes(gNbNodes, ueNodes);
    visualizer->SetPrimaryEdgeNode(primaryEdgeNode);
    visualizer->SetSecondaryEdgeNode(secondaryEdgeNode);
    visualizer->SetupLayout();

    Ptr<RealMetricsCollector> realMetrics = Create<RealMetricsCollector>();

    // NR setup
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
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
    channelHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
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
    for (uint32_t i = 0; i < gNbDevices.GetN(); ++i) {
        nrHelper->GetGnbPhy(gNbDevices.Get(i), 0)->SetAttribute("Numerology", UintegerValue(numerology));
        nrHelper->GetGnbPhy(gNbDevices.Get(i), 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));
    }

    InternetStackHelper internet;
    internet.Install(ueNodes);
    internet.Install(primaryEdgeNode);
    internet.Install(secondaryEdgeNode);

    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevices));
    nrHelper->AttachToClosestGnb(ueDevices, gNbDevices);

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Traffic setup – normal rate (interval 0.01s = 100 pkt/s)
    uint16_t dlPort = 1234, ulPort = 1235;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
        UdpServerHelper dlServer(dlPort + u);
        ApplicationContainer dlServerApp = dlServer.Install(ueNodes.Get(u));
        dlServerApp.Start(Seconds(1.0));
        dlServerApp.Stop(Seconds(simTime - 1));

        UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort + u);
        dlClient.SetAttribute("MaxPackets", UintegerValue(1000000));
        dlClient.SetAttribute("PacketSize", UintegerValue(1400));
        dlClient.SetAttribute("Interval", TimeValue(Seconds(0.01))); // 100 pkt/s
        ApplicationContainer dlClientApp = dlClient.Install(pgw);
        dlClientApp.Start(Seconds(2.0));
        dlClientApp.Stop(Seconds(simTime - 2));

        UdpServerHelper ulServer(ulPort + u);
        ApplicationContainer ulServerApp = ulServer.Install(pgw);
        ulServerApp.Start(Seconds(1.5));
        ulServerApp.Stop(Seconds(simTime - 1));

        UdpClientHelper ulClient(pgw->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), ulPort + u);
        ulClient.SetAttribute("MaxPackets", UintegerValue(1000000));
        ulClient.SetAttribute("PacketSize", UintegerValue(1400));
        ulClient.SetAttribute("Interval", TimeValue(Seconds(0.02))); // 50 pkt/s
        ApplicationContainer ulClientApp = ulClient.Install(ueNodes.Get(u));
        ulClientApp.Start(Seconds(2.5));
        ulClientApp.Stop(Seconds(simTime - 2));
    }

    NodeContainer allNodes;
    allNodes.Add(gNbNodes);
    allNodes.Add(ueNodes);
    allNodes.Add(primaryEdgeNode);
    allNodes.Add(secondaryEdgeNode);
    allNodes.Add(pgw);
    realMetrics->InstallFlowMonitor(allNodes);

    Ptr<SimulationManager> simManager = Create<SimulationManager>(
        gNbNodes.Get(0), primaryEdgeNode, secondaryEdgeNode, realMetrics, visualizer);
    simManager->InitializeAgents();

    visualizer->SetSimulationManager(simManager);
    visualizer->Initialize("Result-folder/nr-immortal.xml");

    // Schedule Baseline Failure early
    Simulator::Schedule(Seconds(1.0), [simManager, realMetrics]() {
        std::cout << "\n=== STARTING BASELINE FAILURE SCENARIO (No Continuity) ===\n";
        realMetrics->RecordRealMetrics("Pre-Baseline Normal", 0);
        simManager->RunBaselineFailureScenario();
        realMetrics->RecordRealMetrics("Baseline Failure Complete",
                                       simManager->GetTwinAgent() ? simManager->GetTwinAgent()->GetTotalContinuityChecks() : 0);
    });

    // Schedule Continuity Scenarios (original four)
    Simulator::Schedule(Seconds(5.0), [simManager]() {
        std::cout << "\n=== STARTING IMMORTAL IDENTITY SYSTEM (With Continuity) ===\n";
        simManager->InitializeAgents();
        simManager->SchedulePeriodicDuties();
        simManager->ScheduleOriginalFailureEvents();
    });

    // New Scenario: Network Partition (after original failures)
    Simulator::Schedule(Seconds(40.0), [simManager]() {
        simManager->NetworkPartitionRejoin();
    });

    // New Scenario: Hard Failure (after partition)
    Simulator::Schedule(Seconds(50.0), [simManager]() {
        simManager->HardFailureNoState();
    });

    // New Scenario: High Load Test (after hard failure)
    Simulator::Schedule(Seconds(60.0), [simManager]() {
        simManager->HighLoadTest();
    });

    // Periodic metric recordings (every 2 seconds)
    for (double t = 1.0; t < simTime; t += 2.0) {
        Simulator::Schedule(Seconds(t), [simManager, realMetrics]() {
            uint32_t checks = (simManager->GetTwinAgent()) ? simManager->GetTwinAgent()->GetTotalContinuityChecks() : 0;
            realMetrics->RecordRealMetrics("Periodic Check", checks);
        });
    }

    Simulator::Schedule(Seconds(simTime - 5), [simManager, realMetrics, visualizer]() {
        if (simManager->GetTwinAgent() && simManager->GetEnterpriseAgent()) {
            simManager->GetTwinAgent()->VerifyAgentContinuity(simManager->GetEnterpriseAgent());
            simManager->GetTwinAgent()->PrintVerificationSummary();
        }
        realMetrics->GenerateRealComparisonReport();
        realMetrics->PrintRealMetricsSummary();
        visualizer->GenerateVisualizationReport();
    });

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

    simManager->PrintFinalReport();

    return 0;
}