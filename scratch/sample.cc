#include <cstdlib>
#include <time.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <map>

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/error-model.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE("WifiTopology");

void ThroughputMonitor(FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
  uint16_t i = 0;

  std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();

  Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier>(fmhelper->GetClassifier());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin();
       stats != flowStats.end(); ++stats)
  {
    Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow(stats->first);

    std::cout << "Flow ID			: " << stats->first << " ; "
              << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress
              << std::endl;
    std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
    std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
    std::cout << "Duration		: "
              << (stats->second.timeLastRxPacket.GetSeconds() -
                  stats->second.timeFirstTxPacket.GetSeconds())
              << std::endl;
    std::cout << "Last Received Packet	: " << stats->second.timeLastRxPacket.GetSeconds()
              << " Seconds" << std::endl;
    std::cout << "Throughput: "
              << stats->second.rxBytes * 8.0 /
                     (stats->second.timeLastRxPacket.GetSeconds() -
                      stats->second.timeFirstTxPacket.GetSeconds()) /
                     1024 / 1024
              << " Mbps" << std::endl;

    i++;

    std::cout << "---------------------------------------------------------------------------"
              << std::endl;
  }

  Simulator::Schedule(Seconds(10), &ThroughputMonitor, fmhelper, flowMon, em);
}

void AverageDelayMonitor(FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
  uint16_t i = 0;

  std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier>(fmhelper->GetClassifier());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin();
       stats != flowStats.end(); ++stats)
  {
    Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow(stats->first);
    std::cout << "Flow ID			: " << stats->first << " ; "
              << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress
              << std::endl;
    std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
    std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
    std::cout << "Duration		: "
              << (stats->second.timeLastRxPacket.GetSeconds() -
                  stats->second.timeFirstTxPacket.GetSeconds())
              << std::endl;
    std::cout << "Last Received Packet	: " << stats->second.timeLastRxPacket.GetSeconds()
              << " Seconds" << std::endl;
    std::cout << "Sum of e2e Delay: " << stats->second.delaySum.GetSeconds() << " s"
              << std::endl;
    std::cout << "Average of e2e Delay: "
              << stats->second.delaySum.GetSeconds() / stats->second.rxPackets << " s"
              << std::endl;

    i++;

    std::cout << "---------------------------------------------------------------------------"
              << std::endl;
  }

  Simulator::Schedule(Seconds(10), &AverageDelayMonitor, fmhelper, flowMon, em);
}

class MyHeader : public Header
{
public:
  MyHeader();
  virtual ~MyHeader();
  void SetData(uint16_t data);
  uint16_t GetData(void) const;
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);
  virtual uint32_t GetSerializedSize(void) const;

private:
  uint16_t m_data;
};

MyHeader::MyHeader()
{
}

MyHeader::~MyHeader()
{
}

TypeId
MyHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::MyHeader").SetParent<Header>().AddConstructor<MyHeader>();
  return tid;
}

TypeId
MyHeader::GetInstanceTypeId(void) const
{
  return GetTypeId();
}

void MyHeader::Print(std::ostream &os) const
{
  os << "data = " << m_data << endl;
}

uint32_t
MyHeader::GetSerializedSize(void) const
{
  return 2;
}

void MyHeader::Serialize(Buffer::Iterator start) const
{
  start.WriteHtonU16(m_data);
}

uint32_t
MyHeader::Deserialize(Buffer::Iterator start)
{
  m_data = start.ReadNtohU16();

  return 2;
}

void MyHeader::SetData(uint16_t data)
{
  m_data = data;
}

uint16_t
MyHeader::GetData(void) const
{
  return m_data;
}

class master : public Application
{
public:
  master(uint16_t port, Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &mappers_ip);
  virtual ~master();

private:
  virtual void StartApplication(void);
  void HandleRead(Ptr<Socket> socket);

  uint16_t port;
  Ipv4InterfaceContainer ip;
  Ipv4InterfaceContainer mappers_ip;
  Ptr<Socket> socket;
};

class client : public Application
{
public:
  client(uint16_t port, Ipv4InterfaceContainer &ip);
  virtual ~client();

private:
  virtual void StartApplication(void);

  uint16_t port;
  Ptr<Socket> socket;
  Ipv4InterfaceContainer ip;
};

class mapper : public Application
{
public:
  mapper(uint16_t id, uint16_t port, Ipv4InterfaceContainer &ip, std::map<int, std::string> mapp);
  virtual ~mapper();

private:
  virtual void StartApplication(void);
  void HandleRead(Ptr<Socket> socket);

  uint16_t id;
  uint16_t port;
  Ipv4InterfaceContainer ip;
  std::map<int, std::string> mapp;
  Ptr<Socket> socket;
};

int main(int argc, char *argv[])
{
  double error = 0.000001;
  string bandwidth = "1Mbps";
  bool verbose = true;
  double duration = 20.0;
  bool tracing = false;
  std::map<int, std::string> Mapper1map;
  std::map<int, std::string> Mapper2map;
  std::map<int, std::string> Mapper3map;
  Mapper1map.insert(std::make_pair(0, "a"));
  Mapper1map.insert(std::make_pair(1, "b"));
  Mapper1map.insert(std::make_pair(2, "c"));
  Mapper1map.insert(std::make_pair(3, "d"));
  Mapper1map.insert(std::make_pair(4, "e"));
  Mapper1map.insert(std::make_pair(5, "f"));
  Mapper1map.insert(std::make_pair(6, "g"));
  Mapper1map.insert(std::make_pair(7, "h"));
  Mapper1map.insert(std::make_pair(8, "i"));
  Mapper2map.insert(std::make_pair(9, "j"));
  Mapper2map.insert(std::make_pair(10, "k"));
  Mapper2map.insert(std::make_pair(11, "l"));
  Mapper2map.insert(std::make_pair(12, "m"));
  Mapper2map.insert(std::make_pair(13, "n"));
  Mapper2map.insert(std::make_pair(14, "o"));
  Mapper2map.insert(std::make_pair(15, "p"));
  Mapper2map.insert(std::make_pair(16, "q"));
  Mapper2map.insert(std::make_pair(17, "r"));
  Mapper3map.insert(std::make_pair(18, "s"));
  Mapper3map.insert(std::make_pair(19, "t"));
  Mapper3map.insert(std::make_pair(20, "u"));
  Mapper3map.insert(std::make_pair(21, "v"));
  Mapper3map.insert(std::make_pair(22, "w"));
  Mapper3map.insert(std::make_pair(23, "x"));
  Mapper3map.insert(std::make_pair(24, "y"));
  Mapper3map.insert(std::make_pair(25, "z"));

  srand(time(NULL));

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);

  cmd.Parse(argc, argv);

  if (verbose)
  {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }

  NodeContainer wifiStaNodeClient;
  wifiStaNodeClient.Create(1);

  NodeContainer wifiStaNodeMaster;
  wifiStaNodeMaster.Create(1);

  NodeContainer wifiStaNodeMapper;
  wifiStaNodeMapper.Create(3);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();

  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));

  NetDeviceContainer staDeviceClient;
  staDeviceClient = wifi.Install(phy, mac, wifiStaNodeClient);

  NetDeviceContainer staDeviceMapper;
  staDeviceMapper = wifi.Install(phy, mac, wifiStaNodeMapper);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer staDeviceMaster;
  staDeviceMaster = wifi.Install(phy, mac, wifiStaNodeMaster);

  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
  em->SetAttribute("ErrorRate", DoubleValue(error));
  phy.SetErrorRateModel("ns3::YansErrorRateModel");

  MobilityHelper mobility;

  mobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
                                DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY",
                                DoubleValue(10.0), "GridWidth", UintegerValue(3), "LayoutType",
                                StringValue("RowFirst"));

  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                            RectangleValue(Rectangle(-50, 50, -50, 50)));
  mobility.Install(wifiStaNodeClient);

  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                            RectangleValue(Rectangle(-50, 50, -50, 50)));
  mobility.Install(wifiStaNodeMapper);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiStaNodeMaster);

  InternetStackHelper stack;
  stack.Install(wifiStaNodeClient);
  stack.Install(wifiStaNodeMaster);
  stack.Install(wifiStaNodeMapper);

  Ipv4AddressHelper address;

  Ipv4InterfaceContainer staNodeClientInterface;
  Ipv4InterfaceContainer staNodesMasterInterface;
  Ipv4InterfaceContainer staNodesMapperInterface;

  address.SetBase("10.1.3.0", "255.255.255.0");
  staNodeClientInterface = address.Assign(staDeviceClient);
  staNodesMasterInterface = address.Assign(staDeviceMaster);
  staNodesMapperInterface = address.Assign(staDeviceMapper);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t port = 1102;
  uint16_t id1 = 0;
  uint16_t id2 = 1;
  uint16_t id3 = 2;
  Ptr<client> clientApp = CreateObject<client>(port, staNodesMasterInterface);
  wifiStaNodeClient.Get(0)->AddApplication(clientApp);
  clientApp->SetStartTime(Seconds(0.0));
  clientApp->SetStopTime(Seconds(duration));

  Ptr<master> masterApp =
      CreateObject<master>(port, staNodesMasterInterface, staNodesMapperInterface);
  wifiStaNodeMaster.Get(0)->AddApplication(masterApp);
  masterApp->SetStartTime(Seconds(0.0));
  masterApp->SetStopTime(Seconds(duration));

  Ptr<mapper> mapperApp1 = CreateObject<mapper>(id1, port, staNodesMapperInterface, Mapper1map);
  wifiStaNodeMapper.Get(0)->AddApplication(mapperApp1);
  mapperApp1->SetStartTime(Seconds(0.0));
  mapperApp1->SetStopTime(Seconds(duration));

  Ptr<mapper> mapperApp2 = CreateObject<mapper>(id2, port, staNodesMapperInterface, Mapper2map);
  wifiStaNodeMapper.Get(1)->AddApplication(mapperApp2);
  mapperApp2->SetStartTime(Seconds(0.0));
  mapperApp2->SetStopTime(Seconds(duration));

  Ptr<mapper> mapperApp3 = CreateObject<mapper>(id3, port, staNodesMapperInterface, Mapper3map);
  wifiStaNodeMapper.Get(2)->AddApplication(mapperApp3);
  mapperApp3->SetStartTime(Seconds(0.0));
  mapperApp3->SetStopTime(Seconds(duration));

  NS_LOG_INFO("Run Simulation");

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  // ThroughputMonitor(&flowHelper, flowMonitor, error);
  // AverageDelayMonitor(&flowHelper, flowMonitor, error);

  Simulator::Stop(Seconds(duration));
  Simulator::Run();

  return 0;
}

client::client(uint16_t port, Ipv4InterfaceContainer &ip) : port(port), ip(ip)
{
  std::srand(time(0));
}

client::~client()
{
}

static void
GenerateTraffic(Ptr<Socket> socket, uint16_t data)
{
  Ptr<Packet> packet = new Packet();
  MyHeader m;
  m.SetData(data);

  packet->AddHeader(m);
  packet->Print(std::cout);
  socket->Send(packet);

  Simulator::Schedule(Seconds(0.1), &GenerateTraffic, socket, rand() % 26);
}

void client::StartApplication(void)
{
  Ptr<Socket> sock = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  InetSocketAddress sockAddr(ip.GetAddress(0), port);
  sock->Connect(sockAddr);

  GenerateTraffic(sock, 0);
}

master::master(uint16_t port, Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &mappers_ip)
    : port(port), ip(ip), mappers_ip(mappers_ip)
{
  std::srand(time(0));
}

master::~master()
{
}

void master::StartApplication(void)
{
  socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  InetSocketAddress local = InetSocketAddress(ip.GetAddress(0), port);
  socket->Bind(local);

  socket->SetRecvCallback(MakeCallback(&master::HandleRead, this));
}

mapper::mapper(uint16_t id, uint16_t port, Ipv4InterfaceContainer &ip, std::map<int, std::string> mapp) : id(id), port(port), ip(ip), mapp(mapp)
{
  std::srand(time(0));
}

mapper::~mapper()
{
}

void mapper::StartApplication(void)
{
  socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  InetSocketAddress local = InetSocketAddress(ip.GetAddress(id), port);
  socket->Bind(local);
  socket->Listen();
  socket->SetRecvCallback(MakeCallback(&mapper::HandleRead, this));
}

void mapper::HandleRead(Ptr<Socket> socket)
{
  Ptr<Packet> packet;

  while ((packet = socket->Recv()))
  {
    if (packet->GetSize() == 0)
    {
      break;
    }

    MyHeader destinationHeader;
    packet->RemoveHeader(destinationHeader);
    uint16_t data = destinationHeader.GetData();
    std::string decoded_data = mapp[data];
    std::cout << decoded_data << std::endl;
    destinationHeader.Print(std::cout);
  }
}

void master::HandleRead(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv()))
  {
    if (packet->GetSize() == 0)
    {
      break;
    }
    for (uint32_t i = 0; i < 3; i++)
    {
      Ptr<Socket> mapperSocket =
          Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
      InetSocketAddress remote = InetSocketAddress(mappers_ip.GetAddress(i), port);
      mapperSocket->Connect(remote);
      mapperSocket->Send(packet);
    }
  }
}