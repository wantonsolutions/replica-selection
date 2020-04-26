/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/multichannel-probe-module.h"
#include "ns3/testmodule-module.h"
//#include "ns3/ipv4-conga-routing-helper.h"
#include "ns3/ipv4-doppelganger-routing-helper.h"
#include "ns3/ipv4-doppelganger-routing.h"

#include <string>
#include <fstream>
#include <stdint.h>

#define TCP 0

#define CROSS_CORE 0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VarClients");

const int K = 4;
const int PODS = K;
const int EDGE = (K/2);
const int EDGES = PODS * EDGE;
const int AGG = (K/2);
const int AGGS = PODS * AGG;
const int CORE = (K/2)*(K/2);
const int NODE = K/2 ;
const int NODES = PODS * EDGE * NODE ;

//GLOBALS
uint32_t CoverNPackets = 100;
float CoverInterval = 0.1;
uint32_t CoverPacketSize = 128;

uint32_t ClientProtocolNPackets = 200;
float ClientProtocolInterval = 0.15;
uint32_t ClientProtocolPacketSize = 256;

double IntervalRatio = .99;

int mode = TCP;

bool debug = false;

std::string ManifestName = "manifest.config";
std::string ProbeName = "default.csv";

const char *CoverNPacketsString = "CoverNPackets";
const char *CoverIntervalString = "CoverInterval";
const char *CoverPacketSizeString = "CoverPacketSize";

const char *ClientProtocolNPacketsString = "ClientProtocolNPackets";
const char *ClientProtocolIntervalString = "ClientProtocolInterval";
const char *ClientProtocolPacketSizeString = "ClientProtocolPacketSize";

const char *IntervalRatioString = "IntervalRatio";

const char *ManifestNameString = "ManifestName";
const char *ProbeNameString = "ProbeName";

const char *DebugString = "Debug";
const char *ModeString = "Mode";

const char *KString = "K";
const char *TopologyString = "Topology";
const char *Topology = "PFatTree";
const char *ParallelString = "Parallel";
//\Globals


//// Custom Routing

void
CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string typeId)
{
  ObjectFactory factory;
  factory.SetTypeId (typeId);
  Ptr<Object> protocol = factory.Create <Object> ();
  node->AggregateObject (protocol);
}
//Derrived from Internet stack helper
static void AddInternetStack (Ptr <Node> node){

      ObjectFactory m_tcpFactory;
      Ipv4RoutingHelper *m_routing;

      m_tcpFactory.SetTypeId ("ns3::TcpL4Protocol");
      Ipv4StaticRoutingHelper staticRouting;
      Ipv4GlobalRoutingHelper globalRouting;
      Ipv4DoppelgangerRoutingHelper doppelgangerRouting;
      Ipv4ListRoutingHelper listRouting;
      //The entire point of this routine is to add this call
      listRouting.Add (doppelgangerRouting, 1);
      //Change complete
      listRouting.Add (staticRouting, 0);
      listRouting.Add (globalRouting, -10);
      m_routing = listRouting.Copy();



      if (node->GetObject<Ipv4> () != 0)
        {
          NS_FATAL_ERROR ("InternetStackHelper::Install (): Aggregating " 
                          "an InternetStack to a node with an existing Ipv4 object");
          return;
        }

      CreateAndAggregateObjectFromTypeId (node, "ns3::ArpL3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv4L3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv4L4Protocol");

      //Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
      //NS_ASSERT (arp);
      //arp->SetAttribute ("RequestJitter", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

      // Set routing
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      Ptr<Ipv4RoutingProtocol> ipv4Routing = m_routing->Create (node);
      ipv4->SetRoutingProtocol (ipv4Routing);

      CreateAndAggregateObjectFromTypeId (node, "ns3::TrafficControlLayer");
      CreateAndAggregateObjectFromTypeId (node, "ns3::UdpL4Protocol");
      node->AggregateObject (m_tcpFactory.Create<Object> ());
      Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory> ();
      node->AggregateObject (factory);

      Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
      Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
      NS_ASSERT (arp);
      NS_ASSERT (tc);
      arp->SetTrafficControl (tc);
}


//----------------------------------------------RPC Client----------------------------------------------------
void InstallRpcClientAttributes(RpcClientHelper *rpcClient, int maxpackets, double interval, int packetsize)
{
  rpcClient->SetAttribute("MaxPackets", UintegerValue(maxpackets));
  rpcClient->SetAttribute("Interval", TimeValue(Seconds(interval)));
  rpcClient->SetAttribute("PacketSize", UintegerValue(packetsize));
}

void InstallRandomRpcClientTransmissions(float start, float stop, int clientIndex, RpcClientHelper *rpcClient, NodeContainer nodes, Address addresses[NODES], uint16_t Ports[NODES], int trafficMatrix[NODES][NODES], uint32_t *global_packets_sent,
std::vector<std::vector<int>> rpcServices, uint64_t * serverLoad)
{
  
  ApplicationContainer clientApps = rpcClient->Install(nodes.Get(clientIndex));
  clientApps.Start(Seconds(start));
  clientApps.Stop(Seconds(stop));
  Ptr<RpcClient> ech = DynamicCast<RpcClient>(clientApps.Get(0));
  ech->SetDistribution(RpcClient::nodist);
  ech->SetIntervalRatio(IntervalRatio);
  ech->SetParallel(false);

  Ptr<RpcClient> uec = DynamicCast<RpcClient>(clientApps.Get(0));

  //Convert Addresses
  ////TODO Start here, trying to convert one set of pointers to another.
  Address *addrs = new Address [NODES];
  uint16_t *ports = new uint16_t [NODES];
  int **tm = new int *[NODES];
  for (int i = 0; i < NODES; i++)
  {
    addrs[i] = addresses[i];
    ports[i] = Ports[i];
    tm[i] = &trafficMatrix[i][0];
  }
  //uec->SetAllAddresses((Address **)(addresses),(uint16_t **)(Ports),PARALLEL,NODES);
  //uec->SetAllAddresses(addrs, ports, tm, PARALLEL, NODES);
  uec->SetAllAddresses(addrs, ports, tm, NODES);
  uec->SetGlobalPackets(global_packets_sent);
  uec->SetRpcServices(rpcServices);
  uec->SetGlobalSeverLoad(serverLoad);
  

  //Fix this should be checked prior to executing
  uec->SetReplicationStrategy(mode);
}

void SetDoppelgangerRoutingParameters(NodeContainer nodes, LoadBallencingStrategy strat, std::vector<std::vector<int>> rpcServices, std::map<uint32_t, uint32_t> ipServerMap, uint64_t * serverLoad) {
  NodeContainer::Iterator i;
  for (i = nodes.Begin(); i != nodes.End(); ++i) {
      NS_LOG_WARN("Installing parameters on" << (*i)->GetId());
      Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();
      Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();

      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting>(routingProtocol);
      int16_t routing_priority=0;
      Ptr<Ipv4DoppelgangerRouting> doppelRouter = DynamicCast<Ipv4DoppelgangerRouting>(list->GetRoutingProtocol(0,routing_priority));
      doppelRouter->SetRpcServices(rpcServices);
      doppelRouter->SetIPServerMap(ipServerMap);
      doppelRouter->SetGlobalServerLoad(serverLoad);
      doppelRouter->SetLoadBallencingStrategy(strat);
      //Start here we need to get the list routing protocol
  }

}

void InstallUniformRpcClientTransmissions(float start, float stop, float gap, int clientIndex, RpcClientHelper *rpcClient, NodeContainer nodes)
{
  for (float base = start; base < stop; base += gap)
  {
    ApplicationContainer clientApps = rpcClient->Install(nodes.Get(clientIndex));
    clientApps.Start(Seconds(base));
    base += gap;
    clientApps.Stop(Seconds(base));
  }
}

void SetupModularRandomRpcClient(float start, float stop, uint16_t Ports[NODES], Address addresses[NODES], int tm[NODES][NODES], NodeContainer nodes, int clientIndex, double interval, int packetsize, int maxpackets, uint32_t * global_packets_sent,
std::vector<std::vector<int>> rpcServices, uint64_t * serverLoad)
{
  //map clients to servers
  //NS_LOG_INFO("Starting Client Packet Size " << packetsize << " interval " << interval << " nPackets " << maxpackets );
  RpcClientHelper rpcClient(addresses[0], int(Ports[0]));
  InstallRpcClientAttributes(&rpcClient, maxpackets, interval, packetsize);
  InstallRandomRpcClientTransmissions(start, stop, clientIndex, &rpcClient, nodes, addresses, Ports, tm, global_packets_sent, rpcServices, serverLoad);
}

void printTM(int tm[NODES][NODES])
{
  printf("\n");
  for (int i = 0; i < NODES; i++)
  {
    for (int j = 0; j < NODES; j++)
    {
      printf("[%2d]", tm[i][j]);
    }
    printf("\n");
  }
}

void zeroTM(int tm[NODES][NODES])
{
  for (int i = 0; i < NODES; i++)
  {
    for (int j = 0; j < NODES; j++)
    {
      tm[i][j] = 0;
    }
  }
}

void populateTrafficMatrix(int tm[NODES][NODES], int pattern)
{
  zeroTM(tm);
  switch (pattern)
  {
  case CROSS_CORE:

    //This relies on the fact that clients are even numbers and servers
    //are odd, ie every client should have a relative server beside
    //them accounting for the (i-1) as the first term of the server
    //index equasion. THe second term adds the index to halfway across
    //the fat tree, the last term mods the server index by the size of the fat-tree.

    for (int i = 0; i < NODES; i++)
    {
      int serverindex = ((i - 1) + ((K * K * K) / 8)) % ((K * K * K) / 4); //TODO Debug this might not be right for all fat trees
      tm[i][serverindex] = 1;
    }
  }
  printTM(tm);
}

void SetupTraffic(float clientStart, float clientStop, float serverStart, float serverStop, int NPackets, float interval, int packetsize, 
int serverport, NodeContainer nodes, int numNodes, int tm[NODES][NODES], Ipv4InterfaceContainer *addresses, int mode, Address secondAddrs[NODES], uint16_t Ports[NODES],
uint32_t * global_packets_sent, 
std::vector<std::vector<int>> rpcReplicas, 
std::vector<std::vector<int>> rpcServices, 
uint64_t * serverLoad) {
  //For reference to this function check out SetupRandomCoverTraffic in pfattree.cc
  //int clientIndex = 0;
  //int serverIndex = 1;
  //Setup Server

  //in this setup we are ignoring trafic matricies completely and just performing a send between a single client and a single server


  //Install an RPC server on each endpoint
  for (int i = 0; i < numNodes; i++) {
    ApplicationContainer serverApps;
    RpcServerHelper rpcServer(serverport);
    serverApps = rpcServer.Install(nodes.Get(i));
		serverApps.Start (Seconds (serverStart));
		serverApps.Stop (Seconds (clientStop));

    //Each server gets exactly 1 RPC to serve

    Ptr<RpcServer> server = DynamicCast<RpcServer>(serverApps.Get(0));
    server->AssignRPC(rpcServices[i]);
    server->SetGlobalLoad(serverLoad);
    server->SetID(i);


  }

  //Setup clients on every node
  for (int i = 0; i < numNodes; i++) {
    SetupModularRandomRpcClient(clientStart, clientStop, Ports, secondAddrs, tm, nodes, i, interval, packetsize, NPackets, global_packets_sent, rpcReplicas,serverLoad);
  }
}


void translateIp(int base, int *a, int *b, int *c, int *d)
{
  *a = base % 256;
  base = base / 256;
  *b = base % 256;
  base = base / 256;
  *c = base % 256;
  base = base / 256;
  *d = base % 256;
  return;
}

//Replication Placement Strategies
//No replication, each server performs exactly 1 RPC
void replicationStrategy_noReplication(std::vector<std::vector<int>> *replicas) {
  for(int i=0;i<NODES;i++) {
    //Each server serves their own ID's RPC
    (*replicas)[i].push_back(i);
  }
}

//Replicate each service so that there are 2 instances of the RPC, and they live on oposite halfs ove the fat tree
void replicationStrategy_crossCoreReplication(std::vector<std::vector<int>> *replicas) {
  for(int i=0;i<NODES;i++) {
    //Each server serves their own ID's RPC
    (*replicas)[i].push_back(i);
    (*replicas)[i].push_back((i + (NODES / 2)) % NODES);
  }
}

int main(int argc, char *argv[])
{
  CommandLine cmd;

  //Default vlaues for command line arguments

  Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

  //Command Line argument debugging code


  cmd.AddValue(CoverNPacketsString, "Number of packets for the cover to echo", CoverNPackets);
  cmd.AddValue(CoverIntervalString, "Interval at which cover traffic broadcasts", CoverInterval);
  cmd.AddValue(CoverPacketSizeString, "The Size of the packet used by the cover traffic", CoverPacketSize);

  cmd.AddValue(ClientProtocolNPacketsString, "Number of packets to echo", ClientProtocolNPackets);
  cmd.AddValue(ClientProtocolIntervalString, "Interval at which a protocol client makes requests", ClientProtocolInterval);
  cmd.AddValue(ClientProtocolPacketSizeString, "Interval at which a protocol client makes requests", ClientProtocolPacketSize);

  cmd.AddValue(IntervalRatioString, "Ratio at which the ratio of client requests increases", IntervalRatio);

  cmd.AddValue(ManifestNameString, "Then name of the ouput manifest (includes all configurations)", ManifestName);
  cmd.AddValue(ProbeNameString, "Then name of the output probe CSV", ProbeName);

  cmd.AddValue(DebugString, "Print all log level info statements for all clients", debug);
  cmd.AddValue(ModeString, "The Composition of the clients ECHO=0 DRED=1 RAID=2", mode);

  cmd.Parse(argc, argv);


  //mode = DRED;
  //
  //Open a file to write out manifest
  std::string manifestFilename = ManifestName;
  std::ios_base::openmode openmode = std::ios_base::out | std::ios_base::trunc;
  //ofstream->open (manifestFilename.c_str (), openmode);
  OutputStreamWrapper StreamWrapper = OutputStreamWrapper(manifestFilename, openmode);
  //StreamWrapper->SetStream(ofstream);
  std::ostream *stream = StreamWrapper.GetStream();

  *stream << CoverNPacketsString << ":" << CoverNPackets << "\n";
  *stream << CoverIntervalString << ":" << CoverInterval << "\n";
  *stream << CoverPacketSizeString << ":" << CoverPacketSize << "\n";
  *stream << ClientProtocolNPacketsString << ":" << ClientProtocolNPackets << "\n";
  *stream << ClientProtocolIntervalString << ":" << ClientProtocolInterval << "\n";
  *stream << ClientProtocolPacketSizeString << ":" << ClientProtocolPacketSize << "\n";
  *stream << IntervalRatioString << ":" << IntervalRatio << "\n";
  *stream << ManifestNameString << ":" << ManifestName << "\n";
  *stream << DebugString << ":" << debug << "\n";
  *stream << ModeString << ":" << mode << "\n";
  *stream << KString << ":" << K << "\n";
  *stream << TopologyString << ":" << Topology << "\n";

  //printf("Client - NPackets %d, baseInterval %f packetSize %d \n",ClientProtocolNPackets,ClientProtocolInterval,ClientProtocolPacketSize);
  //printf("Cover - NPackets %d, baseInterval %f packetSize %d \n",CoverNPackets,CoverInterval,CoverPacketSize);

  Time::SetResolution(Time::NS);
  LogComponentEnable("RpcClientApplication", LOG_LEVEL_WARN);
  LogComponentEnable("DRedundancyClientApplication", LOG_LEVEL_WARN);
  if (debug)
  {
    LogComponentEnable("DRedundancyClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("DRedundancyServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RaidClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RaidServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RpcClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RpcServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("VarClients", LOG_LEVEL_INFO);
  }


  NodeContainer nodes;
  nodes.Create (NODES);

  NodeContainer edge;
  edge.Create(EDGES);

  NodeContainer agg;
  agg.Create(AGGS);

  NodeContainer core;
  core.Create(CORE);

  NodeContainer nc_node2edge[NODES];
  NetDeviceContainer ndc_node2edge[NODES];

  NodeContainer nc_edge2agg[EDGE * AGG * PODS];
  NetDeviceContainer ndc_edge2agg[EDGE * AGG * PODS];

  NodeContainer nc_agg2core[CORE*PODS];
  NetDeviceContainer ndc_agg2core[CORE*PODS];
  

  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      nc_node2edge[n] = NodeContainer(edge.Get(n/(K/2)), nodes.Get(n));
  }

  //connect edges to agg
  for (int pod = 0; pod < PODS; pod++) {
      for (int edgeS = 0; edgeS < EDGE; edgeS++) {
          for (int aggS = 0; aggS < AGG; aggS++) {
              int aggIndex = pod*AGG + aggS;
              int edgeIndex = pod*EDGE + edgeS;
              int link = (pod * (K/2)*(K/2)) + (edgeS * EDGE) + aggS;
              nc_edge2agg[link] = NodeContainer(agg.Get(aggIndex), edge.Get(edgeIndex));
          }
      }
  }

  //connect agg to core
  for (int coreS = 0; coreS < CORE;coreS++) {
      for (int pod = 0; pod < PODS; pod++) {
              nc_agg2core[(coreS * CORE) + pod] = NodeContainer(core.Get(coreS), agg.Get((pod*AGG) + (coreS/AGG)));
      }
  }


  int BaseRate = 1;
  int ModRate = BaseRate;
  std::stringstream datarate;
  datarate << ModRate << "Mbps";
  //printf("Data Rate %s\n", datarate.str().c_str());

  //Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue ("100p"));
  //Config::SetDefault ("ns3::QueueBase::MaxSize", QueueSizeValue(QueueSize("1p")));
  //

  
  //InternetStackHelper stack;
  //stack.Install (nodes);

  //Ipv4CongaRoutingHelper conga;
  //Ipv4ListRoutingHelper list;
  //list.Add (conga, 10);
  //stack.SetRoutingHelper(list);

  //stack.Install (edge);
  //stack.Install (agg);
  //stack.Install (core);


  NodeContainer::Iterator i;
  for (i = nodes.Begin(); i != nodes.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }
  for (i = edge.Begin(); i != edge.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }
  for (i = agg.Begin(); i != agg.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }
  for (i = core.Begin(); i != core.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }

  PointToPointHelper pointToPoint;
  PointToPointHelper pointToPoint2;

  TrafficControlHelper tch;
  int linkrate = 1000;
  int queuedepth = 50;

  pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint.SetDeviceAttribute("DataRate", StringValue(std::to_string(linkrate) + "Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("1.0us"));
  ////
  pointToPoint2.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint2.SetDeviceAttribute("DataRate", StringValue(std::to_string(linkrate) + "Mbps"));
  pointToPoint2.SetChannelAttribute("Delay", StringValue("1.0us"));

  uint16_t handle = tch.SetRootQueueDisc("ns3::FifoQueueDisc");
  tch.AddInternalQueues(handle, 1, "ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));


  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      ndc_node2edge[n] = pointToPoint.Install(nc_node2edge[n]);
  }
  //connect edge to agg
  const int aggC = EDGE * AGG * PODS;
  //const int aggC = 1;
  for (int e=0;e<aggC;e++){
      //printf("edge %d total %d\n",e,aggC);
      //printf("c.GetN == %d\n",nc_edge2agg[e].GetN());
      printf("%d\n",e);
      ndc_edge2agg[e] = pointToPoint.Install(nc_edge2agg[e]);
  }
  //connect agg to cores
  for (int s = 0;s < CORE * PODS;s++) {
      ndc_agg2core[s] = pointToPoint.Install(nc_agg2core[s]);
  }

  //Assign queues AFTER the stack install (not sure why)

  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      tch.Install(ndc_node2edge[n].Get(0));
      tch.Install(ndc_node2edge[n].Get(1));
  }

  printf("Connecting Topology");
  //connect edges to agg
  for (int pod = 0; pod < PODS; pod++) {
      for (int edgeS = 0; edgeS < EDGE; edgeS++) {
          for (int aggS = 0; aggS < AGG; aggS++) {
              //int aggIndex = pod*AGG + aggS;
              //int edgeIndex = pod*EDGE + edgeS;
              int link = (pod * (K/2)*(K/2)) + (edgeS * EDGE) + aggS;
              //int link = (pod * PODS) + (aggIndex) + edgeIndex;
              //printf("link %d, aggIndex %d, edgeIndex %d\n",link,aggIndex,edgeIndex);
              tch.Install(ndc_edge2agg[link].Get(0));
              tch.Install(ndc_edge2agg[link].Get(1));
          }
      }
  }

  //connect agg to core
  for (int coreS = 0; coreS < CORE;coreS++) {
      for (int pod = 0; pod < PODS; pod++) {
              tch.Install(ndc_agg2core[(coreS * CORE) + pod].Get(0));
              tch.Install(ndc_agg2core[(coreS * CORE) + pod].Get(1));
      }
  }


  Ipv4AddressHelper address;
  Ipv4InterfaceContainer node2edge[NODES];
  Ipv4InterfaceContainer edge2agg[EDGE*AGG*PODS];
  Ipv4InterfaceContainer agg2core[CORE*PODS];

  //TODO Assign address as described in the fat tree paper, code for doing so is prototyped in pfattree.c
  address.SetBase("10.1.1.0", "255.255.255.255");
  for (int i=0;i<NODES;i++) {
  	node2edge[i] = address.Assign(ndc_node2edge[i]);
  }
  for (int i=0;i<EDGE*AGG*PODS;i++) {
  	edge2agg[i] = address.Assign(ndc_edge2agg[i]);
  }
  for (int i=0;i<CORE*PODS;i++) {
  	agg2core[i] = address.Assign(ndc_agg2core[i]);
  }

  ////////////////////////////////////////////////////////////////////////////////////
  //Setup Clients
  ///////////////////////////////////////////////////////////////////////////////////
  //int serverport = 9;
  //int clientIndex = 0;
  //int serverIndex = 11;

  int coverserverport = 10;
  float serverStart = 0.0;
  float clientStart = 0.0;
  float clientStop = 0.01;
  float duration = clientStop;

  uint32_t global_packets_sent = 0;

  Address IPS[NODES];
  uint16_t Ports[NODES];
  for (int i = 0; i < NODES; i++)
  {
      IPS[i] = node2edge[i].GetAddress(1);
      Ports[i] = uint16_t(coverserverport);
  }

  //Create Traffic Matrix
  int trafficMatrix[NODES][NODES];
  int pattern = CROSS_CORE;
  populateTrafficMatrix(trafficMatrix, pattern);

  //Create Server RPC database
  std::vector<std::vector<int>> servicesPerServer(NODES, std::vector<int>(0));
  //replicationStrategy_noReplication(&servicesPerServer);
  replicationStrategy_crossCoreReplication(&servicesPerServer);

  //Create Client Database for servers - each index is an RPC. This is an
  //inverted index of the servers. If a client wants to find which servers can
  //service RPC x it can lookup rpcReplicas[x] and get back each of the
  //corresponding servers
  std::vector<std::vector<int>> rpcReplicas(NODES, std::vector<int>(0));
  for(int i=0;i<NODES;i++){
    for (uint j=0;j<servicesPerServer[i].size();j++) {
      rpcReplicas[servicesPerServer[i][j]].push_back(i);
    }
  }

  //Create Server Load Global
  uint64_t* serverLoad = (uint64_t*) malloc(NODES * sizeof(uint64_t));
  for(int i=0;i<NODES;i++) {
    printf("setting server load for server %d\n", i);
    serverLoad[i]=0;
  }
  //Create IP map to server index
  std::map<uint32_t, uint32_t> ipServerMap;
  for (int i=0; i<NODES; i++) {
    Ipv4Address addr = Ipv4Address::ConvertFrom(IPS[i]);
    uint32_t ip_address = addr.Get();
    ipServerMap.insert(std::pair<uint32_t,uint32_t>(ip_address,i));
  }


  Ptr<MultichannelProbe> mcp = CreateObject<MultichannelProbe>(ProbeName);
  mcp->SetAttribute("Interval", StringValue("1s"));
  mcp->AttachAll();
  mcp->Stop(Seconds(duration));

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  //HACK REMOVE
  int PacketSize = 1472;
  //float Rate = 1.0 / ((PARALLEL * (float(linkrate) * (1000000000.0))) / (float(PacketSize) * 8.0));
  float Rate = 1.0 / (((float(linkrate) * (1000000.0))) / (float(PacketSize) * 8.0));
  float MaxInterval = Rate * 1.0;
  printf("Rate %f\n", Rate);
  ClientProtocolInterval = MaxInterval;
  CoverInterval = MaxInterval;

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  clientApps.Start(Seconds(clientStart));
  clientApps.Stop(Seconds(duration));
  serverApps.Start(Seconds(serverStart));
  serverApps.Stop(Seconds(duration));

  Ipv4InterfaceContainer *node2edgePtr = new Ipv4InterfaceContainer[NODES];
  for (int i = 0; i < NODES; i++)
  {
    node2edgePtr[i] = node2edge[i];
  }


  SetupTraffic(
      clientStart,
      duration,
      serverStart,
      duration,
      CoverNPackets,
      CoverInterval,
      CoverPacketSize,
      coverserverport,
      nodes,
      NODES,         //total nodes
      trafficMatrix, //distance

      //&node2pods,
      node2edgePtr,
      mode,
      IPS,
      Ports,
      &global_packets_sent,
      //RPC Globals
      servicesPerServer,
      rpcReplicas,
      serverLoad
  );

  //Assign attributes to routers in network
  LoadBallencingStrategy strat = none;
  printf("setting custom load balencing strats\n");
  SetDoppelgangerRoutingParameters(nodes,strat,servicesPerServer,ipServerMap,serverLoad);
  SetDoppelgangerRoutingParameters(edge,strat,servicesPerServer,ipServerMap,serverLoad);
  SetDoppelgangerRoutingParameters(agg,strat,servicesPerServer,ipServerMap,serverLoad);
  SetDoppelgangerRoutingParameters(core,strat,servicesPerServer,ipServerMap,serverLoad);
  printf("done setting custom load ballencing\n");



  Simulator::Run();
  Simulator::Destroy();
  return 0;
}

//NOTES on how to set up experiments. Each one of the major experiments should
//have some degrees of freedom, and some degrees of configurability.
//
//Things which should be passed in from the command line include - NPackets -
//NormalPacketSize - Client Rate (constant)
//
// The issue of configurability comes in which determining which nodes are
// running which traffic patterns, and what kinds of workloads they are using.
// Composition should be handeled in script. I cannot think of a useful reason
// to configure this early on, at least untill the efficacy of the protocols
// has been demonstrated.
//
//
