/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ipv4-doppelganger-routing.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/flow-id-tag.h"
#include "ns3/rpc-server.h"
#include "ipv4-doppelganger-tag.h"

#include <algorithm>

#define LOOPBACK_PORT 0

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DoppelgangerRouting");

NS_OBJECT_ENSURE_REGISTERED(Ipv4DoppelgangerRouting);

Ipv4DoppelgangerRouting::Ipv4DoppelgangerRouting() : // Parameters
                                                     m_isLeaf(false),
                                                     m_leafId(0),
                                                     m_tdre(MicroSeconds(200)),
                                                     m_alpha(0.2),
                                                     m_C(DataRate("1Gbps")),
                                                     m_Q(3),
                                                     m_agingTime(MilliSeconds(10)),
                                                     m_flowletTimeout(MicroSeconds(50)), // The default value of flowlet timeout is small for experimental purpose
                                                     m_ecmpMode(false),
                                                     // Variables
                                                     m_feedbackIndex(0),
                                                     m_dreEvent(),
                                                     m_agingEvent(),
                                                     m_ipv4(0),
                                                     // doppelganger added parameters
                                                     m_addr(1),
                                                     m_serverLoad(NULL),
                                                     m_serverLoad_update(NULL),
                                                     m_fattree_switch_type(endhost),
                                                     m_packet_redirections(0),
                                                     m_total_packets(0),
                                                     m_fattree_k(0)

{
  NS_LOG_FUNCTION(this);
}

void translateIp(int base, int *a, int *b, int *c, int *d)
{
  *d = base % 256;
  base = base / 256;
  *c = base % 256;
  base = base / 256;
  *b = base % 256;
  base = base / 256;
  *a = base % 256;
  return;
}

uint32_t toIP(int a, int b, int c, int d)
{
  uint32_t total = 0;
  total += a << 24;
  total += b << 16;
  total += c << 8;
  total += d;
  return total;
}

std::string stringIP(uint32_t ip)
{
  int a, b, c, d;
  translateIp(ip, &a, &b, &c, &d);
  std::ostringstream oss;
  oss << a << "." << b << "." << c << "." << d;
  std::string var = oss.str();
  return var;
}

void printIP(uint32_t ip)
{
  int a, b, c, d;
  translateIp(ip, &a, &b, &c, &d);
  NS_LOG_WARN(a << "." << b << "." << c << "." << d);
}

bool SamePod(Ipv4Address first, Ipv4Address second)
{
  int a1, b1, c1, d1, a2, b2, c2, d2;
  translateIp(first.Get(), &a1, &b1, &c1, &d1);
  translateIp(second.Get(), &a2, &b2, &c2, &d2);
  return b1 == b2;
}

bool ClientBelowTor(uint32_t torIp, uint32_t clientIp){
  int a1, b1, c1, d1, a2, b2, c2, d2;
  translateIp(torIp, &a1, &b1, &c1, &d1);
  translateIp(clientIp, &a2, &b2, &c2, &d2);
  return (a1 == a2 && b1 == b2 && c1 == c2);

}

uint32_t OtherPodAddress(Ipv4Address current, int K)
{
  int a, b, c, d;
  translateIp(current.Get(), &a, &b, &c, &d);
  //Simple other pod, it requires knowledge of K though
  b = (b + 1) % K;
  return toIP(a, b, c, d);
}

uint64_t GetInformationDelay(InformationDelayFunction df, uint64_t constant_delay)
{
  switch (df)
  {
  case constant:
    //This requires that we have some measure of distance to the servers that we are going to
    return Simulator::Now().GetNanoSeconds() - constant_delay;
    break;
  case (uniformRandomErrorSTD || 1): //This is some serious bullshit why wont the const value of uniformRandomErrorSTD link to a local package
  {
    uint64_t mean = constant_delay;
    uint64_t std = (mean * 65) / 100;
    uint64_t error_value;

    //Deal with case where mean is 0
    if (std == 0)
    {
      error_value = 0;
    }
    else
    {
      error_value = rand() % std;
    }
    uint64_t final_delay = 0;
    if (rand() % 2)
    {
      final_delay = mean - error_value;
    }
    else
    {
      final_delay = mean + error_value;
    }

    uint64_t now = Simulator::Now().GetNanoSeconds();
    return now - final_delay;
  }
  default:
  {
    NS_LOG_INFO("The Strategy " << df << " is not yet implemented check ipv4-doppleganger-routing.cc (InformationDelayFuction) -- Returning 0 delay");
    return Simulator::Now().GetNanoSeconds();
  }
  }
}

Ipv4DoppelgangerRouting::~Ipv4DoppelgangerRouting()
{
  NS_LOG_FUNCTION(this);
}

TypeId
Ipv4DoppelgangerRouting::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Ipv4DoppelgangerRouting")
                          .SetParent<Object>()
                          .SetGroupName("Internet")
                          .AddConstructor<Ipv4DoppelgangerRouting>();

  return tid;
}

void Ipv4DoppelgangerRouting::SetLeafId(uint32_t leafId)
{
  m_isLeaf = true;
  m_leafId = leafId;
}

void Ipv4DoppelgangerRouting::SetFlowletTimeout(Time timeout)
{
  m_flowletTimeout = timeout;
}

void Ipv4DoppelgangerRouting::SetAlpha(double alpha)
{
  m_alpha = alpha;
}

void Ipv4DoppelgangerRouting::SetTDre(Time time)
{
  m_tdre = time;
}

void Ipv4DoppelgangerRouting::SetLinkCapacity(DataRate dataRate)
{
  m_C = dataRate;
}

void Ipv4DoppelgangerRouting::SetLinkCapacity(uint32_t interface, DataRate dataRate)
{
  m_Cs[interface] = dataRate;
}

void Ipv4DoppelgangerRouting::SetQ(uint32_t q)
{
  m_Q = q;
}

void Ipv4DoppelgangerRouting::AddAddressToLeafIdMap(Ipv4Address addr, uint32_t leafId)
{
  m_ipLeafIdMap[addr] = leafId;
}

void Ipv4DoppelgangerRouting::EnableEcmpMode()
{
  m_ecmpMode = true;
}

void Ipv4DoppelgangerRouting::SetQueueDelta(uint32_t delta) {
  m_delta_queue_difference = delta;
}

uint32_t Ipv4DoppelgangerRouting::GetQueueDelta() {
  return m_delta_queue_difference;
}

void Ipv4DoppelgangerRouting::InitCongestion(uint32_t leafId, uint32_t port, uint32_t congestion)
{
  std::map<uint32_t, std::map<uint32_t, std::pair<Time, uint32_t>>>::iterator itr =
      m_doppelgangerToLeafTable.find(leafId);
  if (itr != m_doppelgangerToLeafTable.end())
  {
    (itr->second)[port] = std::make_pair(Simulator::Now(), congestion);
  }
  else
  {
    std::map<uint32_t, std::pair<Time, uint32_t>> newMap;
    newMap[port] = std::make_pair(Simulator::Now(), congestion);
    m_doppelgangerToLeafTable[leafId] = newMap;
  }
}

void Ipv4DoppelgangerRouting::AddRoute(Ipv4Address network, Ipv4Mask networkMask, uint32_t port)
{
  NS_LOG_LOGIC(this << " Add Doppelganger routing entry: " << network << "/" << networkMask << " would go through port: " << port);
  DoppelgangerRouteEntry doppelgangerRouteEntry;
  doppelgangerRouteEntry.network = network;
  doppelgangerRouteEntry.networkMask = networkMask;
  doppelgangerRouteEntry.port = port;
  m_routeEntryList.push_back(doppelgangerRouteEntry);
}

std::vector<DoppelgangerRouteEntry>
Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntries(Ipv4Address dest)
{
  std::vector<DoppelgangerRouteEntry> doppelgangerRouteEntries;
  std::vector<DoppelgangerRouteEntry>::iterator itr = m_routeEntryList.begin();
  for (; itr != m_routeEntryList.end(); ++itr)
  {
    if ((*itr).networkMask.IsMatch(dest, (*itr).network))
    {
      doppelgangerRouteEntries.push_back(*itr);
    }
  }
  return doppelgangerRouteEntries;
}

std::vector<DoppelgangerRouteEntry>
Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntriesIP(Ipv4Address dest)
{
  std::vector<DoppelgangerRouteEntry> doppelgangerRouteEntries;
  std::vector<DoppelgangerRouteEntry>::iterator itr = m_routeEntryList.begin();
  for (; itr != m_routeEntryList.end(); ++itr)
  {
    if (dest.Get() == (*itr).network.Get())
    {
      doppelgangerRouteEntries.push_back(*itr);
    }
  }
  return doppelgangerRouteEntries;
}

Ptr<Ipv4Route>
Ipv4DoppelgangerRouting::ConstructIpv4Route(uint32_t port, Ipv4Address destAddress)
{
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice(port);
  Ptr<Channel> channel = dev->GetChannel();
  uint32_t otherEnd = (channel->GetDevice(0) == dev) ? 1 : 0;
  Ptr<Node> nextHop = channel->GetDevice(otherEnd)->GetNode();
  uint32_t nextIf = channel->GetDevice(otherEnd)->GetIfIndex();
  Ipv4Address nextHopAddr = nextHop->GetObject<Ipv4>()->GetAddress(nextIf, 0).GetLocal();
  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetOutputDevice(m_ipv4->GetNetDevice(port));
  route->SetGateway(nextHopAddr);
  route->SetSource(m_ipv4->GetAddress(port, 0).GetLocal());
  route->SetDestination(destAddress);
  return route;
}

/* BEGIN DoppleGanger Routing */

void Ipv4DoppelgangerRouting::SetRpcServices(std::vector<std::vector<int>> rpcServices)
{
  m_rpc_server_replicas = rpcServices;
}
void Ipv4DoppelgangerRouting::SetGlobalServerLoad(uint64_t *serverLoad)
{
  m_serverLoad = serverLoad;
}

void Ipv4DoppelgangerRouting::SetLocalServerLoad(std::map<uint32_t,uint64_t> local_load){
  m_local_server_load = local_load;
}

void Ipv4DoppelgangerRouting::InitLocalServerLoad() {
  std::map<uint32_t, uint32_t>::iterator it;
  for ( it = m_server_ip_map.begin(); it != m_server_ip_map.end(); it++ )
  {
      m_local_server_load[it->first] = 0;
  }
}

void Ipv4DoppelgangerRouting::SetGlobalServerLoadUpdate(Time *serverLoad_update)
{
  m_serverLoad_update = serverLoad_update;
}

void Ipv4DoppelgangerRouting::SetGlobalServerLoadLog(std::vector<std::vector<LoadEvent>> *global_load_log)
{
  m_load_log = global_load_log;
}

void Ipv4DoppelgangerRouting::SetIPServerMap(std::map<uint32_t, uint32_t> ip_map)
{
  m_server_ip_map = ip_map;
}

uint64_t Ipv4DoppelgangerRouting::GetDuration() {
  return m_duration;
}

void Ipv4DoppelgangerRouting::SetDuration(uint64_t duration) {
  m_duration = duration;
}

void Ipv4DoppelgangerRouting::InitTorMsgTimerMap(std::map<uint32_t, uint32_t> server_ip_map) {
  std::map<uint32_t,uint64_t> tor_msg_timer_map;

  int a, b, c, d;
  std::map<uint32_t, uint32_t>::iterator it;
  for ( it = server_ip_map.begin(); it != server_ip_map.end(); it++ )
  {
      uint32_t server_ip = it->first;
      translateIp(server_ip, &a, &b, &c, &d);
      uint32_t tor_ip = toIP(a,b,c,1);
      tor_msg_timer_map[tor_ip] = 0;
      //m_local_server_load[it->first] = 0;
  }
  m_tor_msg_timer_map = tor_msg_timer_map;

}

void Ipv4DoppelgangerRouting::SetLoadBalencingStrategy(LoadBalencingStrategy strat)
{
  m_load_balencing_strategy = strat;
}

void Ipv4DoppelgangerRouting::SetAddress(Ipv4Address addr)
{
  m_addr = addr;
}

Ipv4Address Ipv4DoppelgangerRouting::GetAddress()
{
  return m_addr;
}

void Ipv4DoppelgangerRouting::SetFatTreeSwitchType(FatTreeSwitchType ftst)
{
  m_fattree_switch_type = ftst;
}

Ipv4DoppelgangerRouting::FatTreeSwitchType Ipv4DoppelgangerRouting::GetFatTreeSwitchType()
{
  return m_fattree_switch_type;
}

uint64_t Ipv4DoppelgangerRouting::GetPacketRedirections()
{
  return m_packet_redirections;
}

uint64_t Ipv4DoppelgangerRouting::GetTotalPackets()
{
  return m_total_packets;
}

void Ipv4DoppelgangerRouting::SetFatTreeK(uint k)
{
  m_fattree_k = k;
}

uint Ipv4DoppelgangerRouting::GetFatTreeK(void)
{
  return m_fattree_k;
}

void Ipv4DoppelgangerRouting::SetInformationDelayFunction(InformationDelayFunction delay_function)
{
  m_delay_function = delay_function;
}
InformationDelayFunction Ipv4DoppelgangerRouting::GetInformationDelayFunction()
{
  return m_delay_function;
}

void Ipv4DoppelgangerRouting::SetConstantDelay(uint64_t delay)
{
  m_constant_information_delay = delay;
}

uint64_t Ipv4DoppelgangerRouting::GetConstantDelay()
{
  return m_constant_information_delay;
}


void Ipv4DoppelgangerRouting::SetLoadSpreadInterval(uint64_t spread_interval){
  m_load_spread_interval = spread_interval;
}

uint64_t Ipv4DoppelgangerRouting::GetLoadSpreadInterval() {
  return m_load_spread_interval;
}

void Ipv4DoppelgangerRouting::SetGlobalTorQueueDepth(std::map<uint32_t,uint32_t> *tor_service_queue_depth){
  m_tor_service_queue_depth = tor_service_queue_depth;
  //cheating for now, this is also going to intialize the local tor Queue Depth
}

void Ipv4DoppelgangerRouting::SetLocalTorQueueDepth(std::map<uint32_t,uint32_t> tor_service_queue_depth) {
  m_local_tor_service_queue_depth = tor_service_queue_depth;
}

void Ipv4DoppelgangerRouting::UpdateLocalTorQueueDepth(Ipv4DoppelgangerTag tag){
  if (tag.TorQueuesAreNULL()){
    return;
  }
  for(int i=0;i<KTAG/2;i++) {
    m_local_tor_service_queue_depth[tag.GetTorReplica(i)] = tag.GetTorReplicaQueueDepth(i);
  }
  UpdateMsgTimers(tag);
}

void Ipv4DoppelgangerRouting::UpdateMsgTimers(Ipv4DoppelgangerTag tag) {
  //Update information about the timing of the last update
  uint32_t host_ip = tag.GetTorReplica(0); //use any endhost
  int a,b,c,d;
  translateIp(host_ip,&a,&b,&c,&d);
  uint32_t tor_ip = toIP(a,b,c,1);
  m_tor_msg_timer_map[tor_ip] = Simulator::Now().GetNanoSeconds();
}

 void Ipv4DoppelgangerRouting::SetTagTorQueueDepth(Ipv4DoppelgangerTag *tag) {
   uint i=0;

   std::map<uint32_t,uint32_t>::iterator it;

    for ( it = m_local_tor_service_queue_depth.begin(); it != m_local_tor_service_queue_depth.end(); it++ )
    {
        if( ClientBelowTor(m_addr.Get(), it->first) ) {
          tag->SetTorQueueDepth(i,it->first,m_local_tor_service_queue_depth[it->first]);
          i++;
        }
    }
 }


/* END DoppleGanger Routing function additions*/

Ptr<Ipv4Route>
Ipv4DoppelgangerRouting::RouteOutput(Ptr<Packet> packet, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_WARN("ROUTING OUTPUT");
  return 0;
}

uint64_t
Ipv4DoppelgangerRouting::GetInstantenousLoad(int server_id)
{
  Time now = Simulator::Now();
  int64_t time_passed;
  time_passed = now.GetNanoSeconds() - m_serverLoad_update[server_id].GetNanoSeconds();

  //Recalculate load
  int64_t tmp_load = m_serverLoad[server_id]; //use a tmp variable to prevent overflow
  tmp_load -= time_passed;

  //Prevent load from going below 0
  if (tmp_load < 0)
  {
    tmp_load = 0;
  }
  //Update Server Load
  m_serverLoad[server_id] = tmp_load;
  m_serverLoad_update[server_id] = now;

  return m_serverLoad[server_id];
}

uint64_t Ipv4DoppelgangerRouting::GetInformationTime()
{
  return GetInformationDelay(m_delay_function, m_constant_information_delay);
}

//Returns the IP of a minuimum latency replica
uint32_t
Ipv4DoppelgangerRouting::replicaSelectionStrategy_minimumLoad(std::vector<uint32_t> ips)
{
  uint64_t minLoad = UINT64_MAX;
  uint32_t minReplica;

  //TODO make this a command line argument
  switch (m_information_collection_method){
    case instant:
    {
      uint64_t time = GetInformationTime(); //This will likely have to be extended
      for (uint i = 0; i < ips.size(); i++)
      {
        uint32_t replica = m_server_ip_map[ips[i]];
        uint64_t dialated_load = ServerLoadAtTime(replica, time, m_load_log);
        if (dialated_load < minLoad)
        {
          minLoad = dialated_load;
          minReplica = ips[i];
        }
      }
      return minReplica;
    }
    case piggyback:
    {
      NS_LOG_INFO("Piggyback routing");
      for (uint i = 0; i < ips.size(); i++)
      {
        uint32_t load = m_local_server_load[ips[i]];
        if (load < minLoad)
        {
          minLoad = load;
          minReplica = ips[i];
        }
      }
      return minReplica;
    }
    default:
    {
      NS_LOG_WARN("Information collection method " << m_information_collection_method << "is not implemented");
      break;
    }
  }
  NS_LOG_WARN("ERROR information not collected - min routing strategy failed");
  return 0;

}

//Returns the IP of a minuimum latency replica
uint32_t
Ipv4DoppelgangerRouting::replicaSelectionStrategy_minimumLoad_correct_delay(std::vector<uint32_t> ips)
{
  uint64_t minLoad = UINT64_MAX;
  uint32_t minReplica;
  uint64_t information_delay = GetInformationTime(); //This will likely have to be extended

  std::vector<uint64_t> delayed_loads;
  for (uint i = 0; i < ips.size(); i++)
  {
    uint32_t replica = m_server_ip_map[ips[i]];
    uint64_t dialated_load = ServerLoadAtTime(replica, information_delay, m_load_log);
    delayed_loads.push_back(dialated_load);
  }
  //delayed loads represents the load of servers some time in the past
  //uint64_t client_mean_request_generation = 8500; //Change this per experiment
  uint64_t client_mean_request_generation = 11800; //Change this per experiment
  uint64_t server_mean_processing_time = 10000;
  uint64_t rtt = 7000;

  uint64_t core_delay_to_server = (rtt / 4) + information_delay;

  //simulate requets to min
  // calculate the min replica for every time step, where a time step is a guess that a client generated a request
  for (uint64_t time_simulated = 0; time_simulated < core_delay_to_server; time_simulated += client_mean_request_generation)
  {
    minLoad = UINT64_MAX;
    minReplica = 0; // This is intended to cause a crash or many dropped packets if there is a bug ip = 0
    uint minReplica_index;
    for (uint i = 0; i < delayed_loads.size(); i++)
    {
      if (delayed_loads[i] < minLoad)
      {
        minLoad = delayed_loads[i];
        minReplica = ips[i];
        minReplica_index = i;
      }
    }
    //We have the guessed min replica for this point in the simulated time
    delayed_loads[minReplica_index] += server_mean_processing_time;
  }
  return minReplica;
}

std::vector<uint32_t>
Ipv4DoppelgangerRouting::replicaSelectionStrategy_minimumDownwardDistance(std::vector<uint32_t> ips)
{
  std::vector<uint32_t> min_distance_replicas;
  for (uint i = 0; i < ips.size(); ++i)
  {
    int a1, b1, c1, d1, a2, b2, c2, d2;
    translateIp(m_addr.Get(), &a1, &b1, &c1, &d1);
    translateIp(ips[i], &a2, &b2, &c2, &d2);
    switch (m_fattree_switch_type)
    {
    case edge:
      if (a1 == a2 && b1 == b2 && c1 == c2)
      {
        min_distance_replicas.push_back(ips[i]);
      }
      break;
    case agg:
      if (b1 == b2)
      {
        min_distance_replicas.push_back(ips[i]);
      }
      break;
    case core:
      min_distance_replicas.push_back(ips[i]);
      break;
    default:
      NS_LOG_WARN("Min Distance Protocol for this routing is not defined, check how you are defining the switch");
      break;
    }
  }
  if (min_distance_replicas.size() == 0)
  {
    //There is no min distance replica below us in the suppled list. Push the responsibility higher up the tree by returning all ips.
    return ips;
  }
  else
  {
    return min_distance_replicas;
  }
}


bool Ipv4DoppelgangerRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                         UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                         LocalDeliverCallback lcb, ErrorCallback ecb)
{

  //NS_LOG_WARN("HELLO CORE WE ARE NOW ROUTING!!");

  NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);

  Ipv4Header headerPrime = header;

  if ((m_spread_load_info) && (m_fattree_switch_type == edge) && (m_load_balencing_strategy == torQueueDepth) && (!m_started_spreading_info)) {
    NS_LOG_INFO("Load information being spread for the first time on " << stringIP(m_addr.Get()));
    SpreadLoadInfo(headerPrime, ucb);
  }

  Ptr<Packet> packet = ConstCast<Packet>(p);
  Ipv4Address destAddress = headerPrime.GetDestination();

  Ipv4DoppelgangerTag tag;
  bool found = packet->PeekPacketTag(tag);

  //Check that the packet has a doppleganger tag, return if not found
  if (found)
  {
    //printf("Found the packet tag\n");
    NS_LOG_WARN("We have a packet tag!!");
  }
  else
  {
    //printf("where on earth is this pacekt tag\n");
    NS_LOG_WARN("Packet Has no tag, this should not be routed by this router, something is wrong!!!");
    return false;
  }

  std::vector<uint32_t> replicas;
  uint32_t *tag_replicas = tag.GetReplicas();
  for (int i = 0; i < tag.GetReplicaCount(); ++i)
  {
    //NS_LOG_INFO("Pulling replica " << stringIP(tag_replicas[i]) << " from packet tag");
    replicas.push_back(tag_replicas[i]);
  }

  if (tag.GetPacketType() == Ipv4DoppelgangerTag::request)
  {
    //Total packets is now the incorrect name for this variable. It only tracks request packets. Stewart May 17 2020.
    m_total_packets++;
    switch (m_load_balencing_strategy)
    {
    //no load balencing forward packets to end hosts based on source chosen destination
    case none:
      //Don't do anything here, we use source routing in this case
      tag.SetCanRouteDown(true);
      break;

    //minimumLoad - Switches have global instantenous knowledge of server load.
    //They route to minimum replicas based on this information at every step.
    //The destination of the end host can change multiple times per packet.
    case minimumLoad:
    {
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if (min_replica == ipv4Addr.Get() || tag.GetRedirections() >= tag.GetReplicaCount())
      {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      }
      else
      {
        NS_LOG_INFO("the best case replica has changed since source send:" << stringIP(ipv4Addr.Get()) << " --> " << stringIP(min_replica));
        destAddress.Set(min_replica);
        headerPrime.SetDestination(destAddress);
        tag.SetRedirections(tag.GetRedirections() + 1);
        m_packet_redirections++;
      }
      //Tag can be routed down from anywhere
      tag.SetCanRouteDown(true);
      break;
    }

    //coreOnly - Switches perform min load routing, but only if the router in
    //question is a core router. This does not min route packets which would
    //have never crossed the core.
    case coreOnly:
    {
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(replicas);
      //uint32_t min_replica = replicaSelectionStrategy_minimumLoad_correct_delay(replicas);
      //uint32_t min_replica = replicaSelectionStrategy_minimumLoad_with_noise(replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if (min_replica == ipv4Addr.Get())
      {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      }
      else
      {
        if (m_fattree_switch_type == core)
        {
          NS_LOG_INFO("the best case replica has changed since source send:" << stringIP(ipv4Addr.Get()) << " --> " << stringIP(min_replica));
          destAddress.Set(min_replica);
          headerPrime.SetDestination(destAddress);
          tag.SetCanRouteDown(true);
          tag.SetRedirections(tag.GetRedirections() + 1);
          m_packet_redirections++;
        }
        else
        {
          NS_LOG_INFO("Not rerouting because not a core router.. Fix this in the future to be it's own routing protocol");
        }
      }
      tag.SetCanRouteDown(true);
      break;
    }
    case minDistanceMinLoad:
    {
      std::vector<uint32_t> min_distance_down_replicas = replicaSelectionStrategy_minimumDownwardDistance(replicas);
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(min_distance_down_replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if (min_replica == ipv4Addr.Get())
      {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      }
      else
      {
        destAddress.Set(min_replica);
        headerPrime.SetDestination(destAddress);
        tag.SetCanRouteDown(true);
        tag.SetRedirections(tag.GetRedirections() + 1);
        m_packet_redirections++;
      }
      tag.SetCanRouteDown(true);
      break;
    }
    case coreForcedMinDistanceMinLoad:
    {
      NS_LOG_WARN("Running Core Fored Min Distance Min Load (non nessisarily a core router)");
      //All packets must reach the core router before they can be routed down.
      if (m_fattree_switch_type == core)
      {
        NS_LOG_WARN("-------------Reached the Core--------------");
        tag.SetCanRouteDown(true);
      }
      //If a packet is on it's way down the tree use the min load balencing with minimum distance strategy
      if (tag.GetCanRouteDown())
      {
        NS_LOG_WARN("Able to route down the tree");
        std::vector<uint32_t> min_distance_down_replicas = replicaSelectionStrategy_minimumDownwardDistance(replicas);
        //minimum distance replicas
        for (uint i = 0; i < min_distance_down_replicas.size(); i++)
        {
          NS_LOG_WARN("min distance replicas(" << i << "): " << stringIP(min_distance_down_replicas[i]));
        }
        uint32_t min_replica = replicaSelectionStrategy_minimumLoad(min_distance_down_replicas);
        Ipv4Address ipv4Addr = headerPrime.GetDestination();
        if (min_replica == ipv4Addr.Get())
        {
          NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
        }
        else
        {
          NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " the best case replica has changed since source send:" << stringIP(ipv4Addr.Get()) << " --> " << stringIP(min_replica));
          destAddress.Set(min_replica);
          headerPrime.SetDestination(destAddress);
          tag.SetRedirections(tag.GetRedirections() + 1);
          m_packet_redirections++;
        }
      } 
      else
      {
        NS_LOG_WARN("Still Routing UP The tree");
        //Packet is either on a node/edge/agg router, and is going up the tree.
        //We need to make sure that the destination being routed to will at
        //least take the packet to the core routers. To do this we first check
        //if the address in the destination is in the same pod as the router.
        //If it's not we don't need to do anything becasue the regular routing
        //will take care of it. If the packet is in the same pod, we need to
        //pick a cross core address to look up in the routing table to force
        //the extra distance of routing.
        if (SamePod(destAddress, m_addr))
        {
          //modify the destination address
          //do not change the actual packet destination
          NS_LOG_WARN("Matching Pod Routing Up The Tree");
          destAddress.Set(OtherPodAddress(destAddress, GetFatTreeK()));
        }
      }
      break;
    }
    //toronly is identical to core only, with the exception that rerouting only occurs at the tor
    case torOnly:
    {
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if (min_replica == ipv4Addr.Get())
      {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      }
      else
      {
        //.if (m_fattree_switch_type == edge && tag.GetRedirections() <= MAX_REPLICAS) 
        if (m_fattree_switch_type == edge && tag.GetRedirections() <= tag.GetReplicaCount() && m_local_server_load[headerPrime.GetDestination().Get()] > 30000 ) 
        {
          NS_LOG_INFO("the best case replica has changed since source send:" << stringIP(ipv4Addr.Get()) << " --> " << stringIP(min_replica));
          destAddress.Set(min_replica);
          headerPrime.SetDestination(destAddress);
          tag.SetCanRouteDown(true);
          tag.SetRedirections(tag.GetRedirections() + 1);
          m_packet_redirections++;
        }
        else
        {
          NS_LOG_INFO("Not rerouting because not a TOR router.. Fix this in the future to be it's own routing protocol");
        }
      }
      tag.SetCanRouteDown(true);
      break;
    }
    case torQueueDepth:
    {
      if (m_fattree_switch_type == edge) {
        //Find the global min tor queue
        //Choose between local and global routing
        std::map<uint32_t,uint32_t>* tor_queue_depths;
        switch (m_information_collection_method){
          case instant:
          {
            tor_queue_depths=m_tor_service_queue_depth;
          }
          case piggyback:
          {
            tor_queue_depths=&m_local_tor_service_queue_depth;
            //UpdateLocalTorQueueDepth(tag);
          }
        }

        uint32_t min = UINT32_MAX;
        uint32_t min_replica;
        for(uint i=0;i<replicas.size();i++) {
          if((*tor_queue_depths)[replicas[i]] < min) {
            min_replica = replicas[i];
          }
          //Debugging info
          uint32_t depth = (*tor_queue_depths)[replicas[i]];
          NS_LOG_INFO(stringIP(replicas[i]) << " -- " << depth);
        }

        uint32_t selected_replica;

        //Check if the packet has been redirected
        Ipv4Address ipv4Addr = headerPrime.GetDestination();
        if (min_replica == ipv4Addr.Get())
        {
          NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
        } 
        else {
          NS_LOG_INFO("min tor has changed. potentiall prevent additonal routing was: " << stringIP(ipv4Addr.Get()) << " is:  " << stringIP(min_replica));
        }


        //TODO these decisions should be made together
        // Check that a threshold is met before re-routing

        //Only make a routing decision if the client is below you
        if( ClientBelowTor(m_addr.Get(),ipv4Addr.Get())) {
          if ((*tor_queue_depths)[min_replica] + m_delta_queue_difference < (*tor_queue_depths)[ipv4Addr.Get()]) {
            NS_LOG_INFO("&&&&&&&&&&&&&&&&&&&&&&& TOR QUEUE DIFF " << m_delta_queue_difference);
            //printf("TOR QUEUE DEPTH %d\n", m_delta_queue_difference);
            //exit(0);
            selected_replica = min_replica;
          } else {
            selected_replica = ipv4Addr.Get();
          }

          // Over ride desicion if the packet has been redirected allready
          //Only redirect a packet if another replica is available don't send in network indefinatly
          if (tag.GetRedirections() >= 1) {
            selected_replica = ipv4Addr.Get();
          }

          //Set the minimum destination, and increment packet redirections if nessisary
          if (selected_replica != ipv4Addr.Get()) {
            destAddress.Set(selected_replica);
            headerPrime.SetDestination(destAddress);
            tag.SetCanRouteDown(true);
            tag.SetRedirections(tag.GetRedirections() + 1);
            m_packet_redirections++;
          }

          //Increment Tor counter if the request is to be routed down
          if (ClientBelowTor(m_addr.Get(), destAddress.Get())) {

            NS_LOG_INFO("Tor " << stringIP(m_addr.Get()) << " passing new request queue incremend");
            (*tor_queue_depths)[destAddress.Get()]++;

          }
        }

        
        //SetTagTorQueueDepth(&tag);
      }
    }
    break;

    default:
      NS_LOG_WARN("Unable to find load ballencing strategy");
      break;
    }
  }
  else if (tag.GetPacketType() == Ipv4DoppelgangerTag::response)
  {
    NS_LOG_INFO("Received response routing back to client" << stringIP(headerPrime.GetSource().Get()) << " To " << tag.GetHostLoad());
  }
  else if (tag.GetPacketType() == Ipv4DoppelgangerTag::load) 
  {
    Ipv4Address source = headerPrime.GetSource();
    m_local_server_load[source.Get()] = tag.GetHostLoad();
    NS_LOG_INFO("Updated Server load " << stringIP(source.Get()) << " To " << tag.GetHostLoad());
  }
  else if (tag.GetPacketType() == Ipv4DoppelgangerTag::response_piggyback)
  {
    //TODO this should only be used if information is being piggybacked
    Ipv4Address source = headerPrime.GetSource();
    m_local_server_load[source.Get()] = tag.GetHostLoad();
    NS_LOG_INFO("Updated Server load from piggybacked response" << stringIP(source.Get()) << " To " << tag.GetHostLoad());


    //This is the incorrect place to put this becasue it has perfect information, it should not be under piggyback.
    switch (m_load_balencing_strategy)
    {
      case torQueueDepth:
      {
        if (m_fattree_switch_type == edge) {

          std::map<uint32_t,uint32_t>* tor_queue_depths;
          switch (m_information_collection_method){
            case instant:
            {
              tor_queue_depths=m_tor_service_queue_depth;
            }
            case piggyback:
            {
              tor_queue_depths=&m_local_tor_service_queue_depth;
              //UpdateLocalTorQueueDepth(tag);
            }
          }
          NS_LOG_INFO("Checking Tor Queue Depth");
          Ipv4Address source = headerPrime.GetSource();
          //Increment Tor counter if the request is below
          NS_LOG_INFO("Server Address " << stringIP(source.Get()) << " Tor IP " << stringIP(m_addr.Get()));
          if (ClientBelowTor(m_addr.Get(), source.Get())) {
            NS_LOG_INFO("Tor " << stringIP(m_addr.Get()) << " completed request subtracting queue");
            (*tor_queue_depths)[source.Get()]--;
            NS_LOG_INFO("Tor  queu depth now " << (*tor_queue_depths)[source.Get()]);
          }
          //SetTagTorQueueDepth(&tag);

        }
        break;
      }
      default:
      {
        NS_LOG_INFO("Not doing anything special based on load balancing strategy (piggyback)");
      }
    }
  } 
  else if (tag.GetPacketType() == Ipv4DoppelgangerTag::tor_to_tor_load) {
    if (m_load_balencing_strategy == torQueueDepth && m_fattree_switch_type == edge){
          NS_LOG_INFO("RECEIVED LOAD PACKET FROM " << stringIP(header.GetSource().Get()) << "(tor to tor) Updateing local tor queue depth");
          UpdateLocalTorQueueDepth(tag);
          return true;
    } else {
        NS_LOG_INFO("Not doing anything with tor load queue info");
    }
    //return true;
  }
  else
  {
    NS_LOG_INFO("Unknown packet type routing as usual");
  }

  packet->ReplacePacketTag(tag);

  // Packet arrival time
  Time now = Simulator::Now();
  std::vector<DoppelgangerRouteEntry> routeEntries = Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntriesIP(destAddress);
  //Find routing entry for the given IP
  if (routeEntries.size() > 0)
  {
    NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " Entry Hit for " << stringIP(destAddress.Get()) << " Found " << routeEntries.size() << " Entries");
  }
  else
  {
    NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " Entries MISS for " << stringIP(destAddress.Get()));
    return false;
  }

  //Ecmp
  uint32_t selectedPort = routeEntries[rand() % routeEntries.size()].port;

  NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " Setting up route for dest address " << stringIP(header.GetDestination().Get()) << " port " << selectedPort);
  Ptr<Ipv4Route> route = Ipv4DoppelgangerRouting::ConstructIpv4Route(selectedPort, destAddress);

  //Check channel state
  Ptr<Channel> chan = idev->GetChannel();

  //ucb (route, packet, header);
  ucb(route, packet, headerPrime);

  return true;
}

void Ipv4DoppelgangerRouting::SpreadLoadInfo(Ipv4Header header, UnicastForwardCallback ucb) {
  m_started_spreading_info = true;
  NS_LOG_INFO("<<<<<<<<<<<<<<<<<<SPREADING TOR LOAD INFO>>>>>>>>>>>>>>>>>> On Host " << stringIP(m_addr.Get()) << " At Time: " << Simulator::Now().GetNanoSeconds());
  std::map<uint32_t, uint64_t>::iterator it;
  for ( it = m_tor_msg_timer_map.begin(); it != m_tor_msg_timer_map.end(); it++ )
  {

      NS_LOG_INFO("Inspecting TOR IP " << stringIP(it->first));
      uint32_t tor_ip = it->first;
      uint64_t last_msg = it->second;

      if (tor_ip == m_addr.Get()) {
        NS_LOG_INFO("Tor Info to self inspected... I don't need to update myself... skipping");
        continue;
      }

      //if ((Simulator::Now().GetNanoSeconds() - last_msg) >= m_load_spread_interval) {
      if (true) {
        NS_LOG_INFO(">>>>>>>>>> Last Message to " << stringIP(tor_ip) << " is " << Simulator::Now().GetNanoSeconds() - last_msg << " ns out of date sending info");

        //create a fake dest address
        //Set it to the first host under the selected TOR
        int a, b, c, d;
        translateIp(tor_ip,&a,&b,&c,&d);
        Ipv4Address destAddress = Ipv4Address(toIP(a,b,c,2));
        

        Ptr<Packet> p;
        uint32_t packet_size = 128;
        p = Create<Packet>(packet_size);


        Ipv4DoppelgangerTag ipv4DoppelgangerTag;
        
        // PacketID is an analog for sequence number, for now request are a single packet so always set to 0
        ipv4DoppelgangerTag.SetCanRouteDown(true);
        ipv4DoppelgangerTag.SetPacketType(Ipv4DoppelgangerTag::tor_to_tor_load);
        ipv4DoppelgangerTag.SetPacketID(0); 
        ipv4DoppelgangerTag.SetHostSojournTime(0);
        ipv4DoppelgangerTag.SetRedirections(0);
        ipv4DoppelgangerTag.SetTorQueuesNULL();

        // Generate the location of the next reuqest
        ipv4DoppelgangerTag.SetRequestID(0);
        for (uint i=0;i<MAX_REPLICAS;i++) {
          ipv4DoppelgangerTag.SetReplica(i,destAddress.Get());
        }
        SetTagTorQueueDepth(&ipv4DoppelgangerTag);
        p->AddPacketTag(ipv4DoppelgangerTag);


        std::vector<DoppelgangerRouteEntry> routeEntries = Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntriesIP(destAddress);
        //Find routing entry for the given IP
        if (routeEntries.size() > 0)
        {
          NS_LOG_WARN("<tor_load> Host: " << stringIP(m_addr.Get()) << " Entry Hit for " << stringIP(destAddress.Get()) << " Found " << routeEntries.size() << " Entries");
        }
        else
        {
          NS_LOG_WARN("<tor_load> Host: " << stringIP(m_addr.Get()) << " Entries MISS for " << stringIP(destAddress.Get()));
        }

        //Ecmp
        uint32_t selectedPort = routeEntries[rand() % routeEntries.size()].port;
        Ptr<Ipv4Route> route = Ipv4DoppelgangerRouting::ConstructIpv4Route(selectedPort, destAddress);
        header.SetDestination(destAddress);

        //ucb (route, packet, header);
        ucb(route, p, header);
        m_tor_msg_timer_map[tor_ip] = Simulator::Now().GetNanoSeconds();
      } else {
        NS_LOG_INFO("Not sending to " << stringIP(tor_ip) << " had a message sent just " << Simulator::Now().GetNanoSeconds() - last_msg << " ns ago");
      }
  }
  if (Simulator::Now().GetNanoSeconds() < NanoSeconds(m_duration)) {
    Simulator::Schedule(NanoSeconds((m_load_spread_interval)), &Ipv4DoppelgangerRouting::SpreadLoadInfo, this, header, ucb);
  }
}

void Ipv4DoppelgangerRouting::NotifyInterfaceUp(uint32_t interface)
{
}

void Ipv4DoppelgangerRouting::NotifyInterfaceDown(uint32_t interface)
{
}

void Ipv4DoppelgangerRouting::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

void Ipv4DoppelgangerRouting::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

void Ipv4DoppelgangerRouting::SetIpv4(Ptr<Ipv4> ipv4)
{
  NS_LOG_LOGIC(this << "Setting up Ipv4: " << ipv4);
  NS_ASSERT(m_ipv4 == 0 && ipv4 != 0);
  m_ipv4 = ipv4;
}
/*
void
Ipv4DoppelgangerRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
  printf("routing-table-stub");
}
*/

void Ipv4DoppelgangerRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
}

void Ipv4DoppelgangerRouting::DoDispose(void)
{
  std::map<uint32_t, Flowlet *>::iterator itr = m_flowletTable.begin();
  for (; itr != m_flowletTable.end(); ++itr)
  {
    delete (itr->second);
  }
  m_dreEvent.Cancel();
  m_agingEvent.Cancel();
  m_ipv4 = 0;
  Ipv4RoutingProtocol::DoDispose();
}

uint32_t
Ipv4DoppelgangerRouting::UpdateLocalDre(const Ipv4Header &header, Ptr<Packet> packet, uint32_t port)
{
  uint32_t X = 0;
  std::map<uint32_t, uint32_t>::iterator XItr = m_XMap.find(port);
  if (XItr != m_XMap.end())
  {
    X = XItr->second;
  }
  uint32_t newX = X + packet->GetSize() + header.GetSerializedSize();
  NS_LOG_LOGIC(this << " Update local dre, new X: " << newX);
  m_XMap[port] = newX;
  return newX;
}

void Ipv4DoppelgangerRouting::DreEvent()
{
  bool moveToIdleStatus = true;

  std::map<uint32_t, uint32_t>::iterator itr = m_XMap.begin();
  for (; itr != m_XMap.end(); ++itr)
  {
    uint32_t newX = itr->second * (1 - m_alpha);
    itr->second = newX;
    if (newX != 0)
    {
      moveToIdleStatus = false;
    }
  }

  NS_LOG_LOGIC(this << " Dre event finished, the dre table is now: ");
  Ipv4DoppelgangerRouting::PrintDreTable();

  if (!moveToIdleStatus)
  {
    m_dreEvent = Simulator::Schedule(m_tdre, &Ipv4DoppelgangerRouting::DreEvent, this);
  }
  else
  {
    NS_LOG_LOGIC(this << " Dre event goes into idle status");
  }
}

void Ipv4DoppelgangerRouting::AgingEvent()
{
  bool moveToIdleStatus = true;
  std::map<uint32_t, std::map<uint32_t, std::pair<Time, uint32_t>>>::iterator itr =
      m_doppelgangerToLeafTable.begin();
  for (; itr != m_doppelgangerToLeafTable.end(); ++itr)
  {
    std::map<uint32_t, std::pair<Time, uint32_t>>::iterator innerItr =
        (itr->second).begin();
    for (; innerItr != (itr->second).end(); ++innerItr)
    {
      if (Simulator::Now() - (innerItr->second).first > m_agingTime)
      {
        (innerItr->second).second = 0;
      }
      else
      {
        moveToIdleStatus = false;
      }
    }
  }
  std::map<uint32_t, std::map<uint32_t, FeedbackInfo>>::iterator itr2 =
      m_doppelgangerFromLeafTable.begin();
  for (; itr2 != m_doppelgangerFromLeafTable.end(); ++itr2)
  {
    std::map<uint32_t, FeedbackInfo>::iterator innerItr2 =
        (itr2->second).begin();
    for (; innerItr2 != (itr2->second).end(); ++innerItr2)
    {
      if (Simulator::Now() - (innerItr2->second).updateTime > m_agingTime)
      {
        (itr2->second).erase(innerItr2);
        if ((itr2->second).empty())
        {
          m_doppelgangerFromLeafTable.erase(itr2);
        }
      }
      else
      {
        moveToIdleStatus = false;
      }
    }
  }

  if (!moveToIdleStatus)
  {
    m_agingEvent = Simulator::Schedule(m_agingTime / 4, &Ipv4DoppelgangerRouting::AgingEvent, this);
  }
  else
  {
    NS_LOG_LOGIC(this << " Aging event goes into idle status");
  }
}

uint32_t
Ipv4DoppelgangerRouting::QuantizingX(uint32_t interface, uint32_t X)
{
  DataRate c = m_C;
  std::map<uint32_t, DataRate>::iterator itr = m_Cs.find(interface);
  if (itr != m_Cs.end())
  {
    c = itr->second;
  }
  double ratio = static_cast<double>(X * 8) / (c.GetBitRate() * m_tdre.GetSeconds() / m_alpha);
  NS_LOG_LOGIC("ratio: " << ratio);
  return static_cast<uint32_t>(ratio * std::pow(2, m_Q));
}

void Ipv4DoppelgangerRouting::PrintDoppelgangerToLeafTable()
{
  /*
  std::ostringstream oss;
  oss << "===== DoppelgangerToLeafTable For Leaf: " << m_leafId <<"=====" << std::endl;
  std::map<uint32_t, std::map<uint32_t, uint32_t> >::iterator itr = m_doppelgangerToLeafTable.begin ();
  for ( ; itr != m_doppelgangerToLeafTable.end (); ++itr )
  {
    oss << "Leaf ID: " << itr->first << std::endl<<"\t";
    std::map<uint32_t, uint32_t>::iterator innerItr = (itr->second).begin ();
    for ( ; innerItr != (itr->second).end (); ++innerItr)
    {
      oss << "{ port: "
          << innerItr->first << ", ce: "  << (innerItr->second)
          << " } ";
    }
    oss << std::endl;
  }
  oss << "============================";
  NS_LOG_LOGIC (oss.str ());
*/
}

void Ipv4DoppelgangerRouting::PrintDoppelgangerFromLeafTable()
{
  /*
  std::ostringstream oss;
  oss << "===== DoppelgangerFromLeafTable For Leaf: " << m_leafId << "=====" <<std::endl;
  std::map<uint32_t, std::map<uint32_t, FeedbackInfo> >::iterator itr = m_doppelgangerFromLeafTable.begin ();
  for ( ; itr != m_doppelgangerFromLeafTable.end (); ++itr )
  {
    oss << "Leaf ID: " << itr->first << std::endl << "\t";
    std::map<uint32_t, FeedbackInfo>::iterator innerItr = (itr->second).begin ();
    for ( ; innerItr != (itr->second).end (); ++innerItr)
    {
      oss << "{ port: "
          << innerItr->first << ", ce: "  << (innerItr->second).ce
          << ", change: " << (innerItr->second).change
          << " } ";
    }
    oss << std::endl;
  }
  oss << "==============================";
  NS_LOG_LOGIC (oss.str ());
*/
}

void Ipv4DoppelgangerRouting::PrintFlowletTable()
{
  /*
  std::ostringstream oss;
  oss << "===== Flowlet For Leaf: " << m_leafId << "=====" << std::endl;
  std::map<uint32_t, Flowlet*>::iterator itr = m_flowletTable.begin ();
  for ( ; itr != m_flowletTable.end(); ++itr )
  {
    oss << "flowId: " << itr->first << std::endl << "\t"
        << "port: " << (itr->second)->port << "\t"
        << "activeTime" << (itr->second)->activeTime << std::endl;
  }
  oss << "===================";
  NS_LOG_LOGIC (oss.str ());
*/
}

void Ipv4DoppelgangerRouting::PrintDreTable()
{
  /*
  std::ostringstream oss;
  std::string switchType = m_isLeaf == true ? "leaf switch" : "spine switch";
  oss << "==== Local Dre for " << switchType << " ====" <<std::endl;
  std::map<uint32_t, uint32_t>::iterator itr = m_XMap.begin ();
  for ( ; itr != m_XMap.end (); ++itr)
  {
    oss << "port: " << itr->first <<
      ", X: " << itr->second <<
      ", Quantized X: " << Ipv4DoppelgangerRouting::QuantizingX (itr->second) <<std::endl;
  }
  oss << "=================================";
  NS_LOG_LOGIC (oss.str ());
*/
}

} // namespace ns3
