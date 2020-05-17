/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ipv4-doppelganger-routing.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/flow-id-tag.h"
#include "ipv4-doppelganger-tag.h"

#include <algorithm>

#define LOOPBACK_PORT 0

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4DoppelgangerRouting");

NS_OBJECT_ENSURE_REGISTERED (Ipv4DoppelgangerRouting);

Ipv4DoppelgangerRouting::Ipv4DoppelgangerRouting ():
    // Parameters
    m_isLeaf (false),
    m_leafId (0),
    m_tdre (MicroSeconds(200)),
    m_alpha (0.2),
    m_C (DataRate("1Gbps")),
    m_Q (3),
    m_agingTime (MilliSeconds (10)),
    m_flowletTimeout (MicroSeconds(50)), // The default value of flowlet timeout is small for experimental purpose
    m_ecmpMode (false),
    // Variables
    m_feedbackIndex (0),
    m_dreEvent (),
    m_agingEvent (),
    m_ipv4 (0),
    // doppelganger added parameters
    m_addr(1),
    m_serverLoad(NULL),
    m_serverLoad_update(NULL),
    m_fattree_switch_type(endhost),
    m_packet_redirections(0),
    m_total_packets(0)

{
  NS_LOG_FUNCTION (this);
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

std::string stringIP(uint32_t ip) {
  int a,b,c,d;
  translateIp(ip,&a,&b,&c,&d);
  std::ostringstream oss;
  oss << a << "." << b << "." << c << "." << d;
  std::string var = oss.str();
  return var;
}

void printIP(uint32_t ip) {
  int a,b,c,d;
  translateIp(ip,&a,&b,&c,&d);
  NS_LOG_WARN(a << "."<< b << "." << c << "." << d );
}

Ipv4DoppelgangerRouting::~Ipv4DoppelgangerRouting ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
Ipv4DoppelgangerRouting::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::Ipv4DoppelgangerRouting")
      .SetParent<Object>()
      .SetGroupName ("Internet")
      .AddConstructor<Ipv4DoppelgangerRouting> ();

  return tid;
}

void
Ipv4DoppelgangerRouting::SetLeafId (uint32_t leafId)
{
  m_isLeaf = true;
  m_leafId = leafId;
}

void
Ipv4DoppelgangerRouting::SetFlowletTimeout (Time timeout)
{
  m_flowletTimeout = timeout;
}

void
Ipv4DoppelgangerRouting::SetAlpha (double alpha)
{
  m_alpha = alpha;
}

void
Ipv4DoppelgangerRouting::SetTDre (Time time)
{
  m_tdre = time;
}

void
Ipv4DoppelgangerRouting::SetLinkCapacity (DataRate dataRate)
{
  m_C = dataRate;
}

void
Ipv4DoppelgangerRouting::SetLinkCapacity (uint32_t interface, DataRate dataRate)
{
  m_Cs[interface] = dataRate;
}

void
Ipv4DoppelgangerRouting::SetQ (uint32_t q)
{
  m_Q = q;
}

void
Ipv4DoppelgangerRouting::AddAddressToLeafIdMap (Ipv4Address addr, uint32_t leafId)
{
  m_ipLeafIdMap[addr] = leafId;
}

void
Ipv4DoppelgangerRouting::EnableEcmpMode ()
{
  m_ecmpMode = true;
}

void
Ipv4DoppelgangerRouting::InitCongestion (uint32_t leafId, uint32_t port, uint32_t congestion)
{
  std::map<uint32_t, std::map<uint32_t, std::pair<Time, uint32_t> > >::iterator itr =
      m_doppelgangerToLeafTable.find(leafId);
  if (itr != m_doppelgangerToLeafTable.end ())
  {
    (itr->second)[port] = std::make_pair(Simulator::Now (), congestion);
  }
  else
  {
    std::map<uint32_t, std::pair<Time, uint32_t> > newMap;
    newMap[port] = std::make_pair(Simulator::Now (), congestion);
    m_doppelgangerToLeafTable[leafId] = newMap;
  }
}

void
Ipv4DoppelgangerRouting::AddRoute (Ipv4Address network, Ipv4Mask networkMask, uint32_t port)
{
  NS_LOG_LOGIC (this << " Add Doppelganger routing entry: " << network << "/" << networkMask << " would go through port: " << port);
  DoppelgangerRouteEntry doppelgangerRouteEntry;
  doppelgangerRouteEntry.network = network;
  doppelgangerRouteEntry.networkMask = networkMask;
  doppelgangerRouteEntry.port = port;
  m_routeEntryList.push_back (doppelgangerRouteEntry);
}

std::vector<DoppelgangerRouteEntry>
Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntries (Ipv4Address dest)
{
  std::vector<DoppelgangerRouteEntry> doppelgangerRouteEntries;
  std::vector<DoppelgangerRouteEntry>::iterator itr = m_routeEntryList.begin ();
  for ( ; itr != m_routeEntryList.end (); ++itr)
  {
    if((*itr).networkMask.IsMatch(dest, (*itr).network))
    {
      doppelgangerRouteEntries.push_back (*itr);
    }
  }
  return doppelgangerRouteEntries;
}

std::vector<DoppelgangerRouteEntry>
Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntriesIP (Ipv4Address dest)
{
  std::vector<DoppelgangerRouteEntry> doppelgangerRouteEntries;
  std::vector<DoppelgangerRouteEntry>::iterator itr = m_routeEntryList.begin ();
  for ( ; itr != m_routeEntryList.end (); ++itr)
  {
    if (dest.Get() == (*itr).network.Get())
    {
      doppelgangerRouteEntries.push_back (*itr);
    }
  }
  return doppelgangerRouteEntries;
}

Ptr<Ipv4Route>
Ipv4DoppelgangerRouting::ConstructIpv4Route (uint32_t port, Ipv4Address destAddress)
{
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice (port);
  Ptr<Channel> channel = dev->GetChannel ();
  uint32_t otherEnd = (channel->GetDevice (0) == dev) ? 1 : 0;
  Ptr<Node> nextHop = channel->GetDevice (otherEnd)->GetNode ();
  uint32_t nextIf = channel->GetDevice (otherEnd)->GetIfIndex ();
  Ipv4Address nextHopAddr = nextHop->GetObject<Ipv4>()->GetAddress(nextIf,0).GetLocal();
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetOutputDevice (m_ipv4->GetNetDevice (port));
  route->SetGateway (nextHopAddr);
  route->SetSource (m_ipv4->GetAddress (port, 0).GetLocal ());
  route->SetDestination (destAddress);
  return route;
}

/* BEGIN DoppleGanger Routing */

  void Ipv4DoppelgangerRouting::SetRpcServices(std::vector<std::vector<int>> rpcServices){
    m_rpc_server_replicas = rpcServices;
  }
  void Ipv4DoppelgangerRouting::SetGlobalServerLoad(uint64_t *serverLoad){
    m_serverLoad = serverLoad;
  }

  void Ipv4DoppelgangerRouting::SetGlobalServerLoadUpdate(Time *serverLoad_update){
    m_serverLoad_update = serverLoad_update;
  }

  void Ipv4DoppelgangerRouting::SetIPServerMap(std::map<uint32_t,uint32_t> ip_map){
    m_server_ip_map = ip_map;
  }

  void Ipv4DoppelgangerRouting::SetLoadBalencingStrategy(LoadBalencingStrategy strat) {
    m_load_balencing_strategy = strat;
  }

  void Ipv4DoppelgangerRouting::SetAddress(Ipv4Address addr) {
    m_addr = addr;
  }

  Ipv4Address Ipv4DoppelgangerRouting::GetAddress() {
    return m_addr;
  }

  void Ipv4DoppelgangerRouting::SetFatTreeSwitchType(FatTreeSwitchType ftst) {
    m_fattree_switch_type = ftst;
  }

  Ipv4DoppelgangerRouting::FatTreeSwitchType Ipv4DoppelgangerRouting::GetFatTreeSwitchType() {
    return m_fattree_switch_type;
  }

  uint64_t Ipv4DoppelgangerRouting::GetPacketRedirections() {
    return m_packet_redirections;
  }

  uint64_t Ipv4DoppelgangerRouting::GetTotalPackets() {
    return m_total_packets;
  }
/* END DoppleGanger Routing function additions*/

Ptr<Ipv4Route>
Ipv4DoppelgangerRouting::RouteOutput (Ptr<Packet> packet, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_WARN("ROUTING OUTPUT");
  return 0;
}

uint64_t
Ipv4DoppelgangerRouting::GetInstantenousLoad(int server_id) {
  Time now = Simulator::Now();
  int64_t time_passed;
  time_passed = now.GetNanoSeconds() - m_serverLoad_update[server_id].GetNanoSeconds();

  //Recalculate load
  int64_t tmp_load = m_serverLoad[server_id]; //use a tmp variable to prevent overflow
  tmp_load -= time_passed;

  //Prevent load from going below 0
  if (tmp_load < 0) {
    tmp_load = 0;
  }
  //Update Server Load
  m_serverLoad[server_id] = tmp_load;
  m_serverLoad_update[server_id] = now;

  return m_serverLoad[server_id];
}


//Returns the IP of a minuimum latency replica
 uint32_t
 Ipv4DoppelgangerRouting::replicaSelectionStrategy_minimumLoad(std::vector<uint32_t> ips){
   uint64_t minLoad = UINT64_MAX;
   uint32_t minReplica;
   for (uint i = 0; i < ips.size();i++) {
     uint32_t serverIndex = m_server_ip_map[ips[i]];
     if (GetInstantenousLoad(serverIndex) < minLoad){
       minLoad = GetInstantenousLoad(serverIndex);
       minReplica = ips[i];
     }
   }
   return minReplica;
 }

 std::vector<uint32_t>
 Ipv4DoppelgangerRouting::replicaSelectionStrategy_minimumDownwardDistance(std::vector<uint32_t> ips) {
   std::vector<uint32_t> min_distance_replicas;
   for(uint i=0;i<ips.size();++i){
     int a1,b1,c1,d1,a2,b2,c2,d2;
     translateIp(m_addr.Get(),&a1,&b1,&c1,&d1);
     translateIp(ips[i],&a2,&b2,&c2,&d2);
     switch (m_fattree_switch_type) {
       case edge:
        if(c1==c2) {
          min_distance_replicas.push_back(ips[i]);
        }
       break;
       case agg:
        if(b1 == b2){
          min_distance_replicas.push_back(ips[i]);
        }
       break;
       case core:
        min_distance_replicas.push_back(ips[i]);
       break;
       default:
        NS_LOG_WARN("Min Distance Protocol for this routing is not defined, check how you are defining the switch");
     }
   }
   if (min_distance_replicas.size() == 0) {
     //There is no min distance replica below us in the suppled list. Push the responsibility higher up the tree by returning all ips.
     return ips;
   } else {
    return min_distance_replicas;
   }
 }

bool
Ipv4DoppelgangerRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb)
{

  //NS_LOG_WARN("HELLO CORE WE ARE NOW ROUTING!!");

  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);

  m_total_packets++;
  Ipv4Header headerPrime = header;

  Ptr<Packet> packet = ConstCast<Packet> (p);
  Ipv4Address destAddress = headerPrime.GetDestination();

  Ipv4DoppelgangerTag ipv4DoppelgangerTag;
  bool found = packet->PeekPacketTag(ipv4DoppelgangerTag);

  if (found) {
    //printf("Found the packet tag\n");
    NS_LOG_WARN("We have a packet tag!!");
  } else {
    //printf("where on earth is this pacekt tag\n");
    NS_LOG_WARN("Packet Has no tag");
  }


  std::vector<uint32_t> replicas;
  uint32_t * tag_replicas = ipv4DoppelgangerTag.GetReplicas();
  for (int i=0;i<MAX_REPLICAS;++i){
    replicas.push_back(tag_replicas[i]);
  }
  switch (m_load_balencing_strategy) {
    //no load balencing forward packets to end hosts based on source chosen destination
    case none:
      //Don't do anything here, we use source routing in this case
      break;

    //minimumLoad - Switches have global instantenous knowledge of server load.
    //They route to minimum replicas based on this information at every step.
    //The destination of the end host can change multiple times per packet.
    case minimumLoad: {
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if(min_replica == ipv4Addr.Get()) {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      } else {
          NS_LOG_INFO("the best case replica has changed since source send:" << stringIP(ipv4Addr.Get()) << " --> " << stringIP(min_replica));
          destAddress.Set(min_replica);
          headerPrime.SetDestination(destAddress);
          m_packet_redirections++;
      }
      break;
    }
     
    //coreOnly - Switches perform min load routing, but only if the router in
    //question is a core router. This does not min route packets which would
    //have never crossed the core.
    case coreOnly: {
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if(min_replica == ipv4Addr.Get()) {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      } else {
        if(m_fattree_switch_type == core) {
          NS_LOG_INFO("the best case replica has changed since source send:" << stringIP(ipv4Addr.Get()) << " --> " << stringIP(min_replica));
          destAddress.Set(min_replica);
          headerPrime.SetDestination(destAddress);
          m_packet_redirections++;
        } else {
          NS_LOG_INFO("Not rerouting because not a core router.. Fix this in the future to be it's own routing protocol");
        }
      }
      break;
    }
    case minDistanceMinLoad: {
      std::vector<uint32_t> min_distance_down_replicas = replicaSelectionStrategy_minimumDownwardDistance(replicas);
      uint32_t min_replica = replicaSelectionStrategy_minimumLoad(min_distance_down_replicas);
      Ipv4Address ipv4Addr = headerPrime.GetDestination();
      if(min_replica == ipv4Addr.Get()) {
        NS_LOG_INFO("replica is the same as the min! Replica: " << stringIP(min_replica));
      } else {
        destAddress.Set(min_replica);
        headerPrime.SetDestination(destAddress);
        m_packet_redirections++;
      }
      break;
    
    }


    default:
      NS_LOG_WARN("Unable to find load ballencing strategy");
      break;
  }



  // Packet arrival time
  Time now = Simulator::Now ();
  std::vector<DoppelgangerRouteEntry> routeEntries = Ipv4DoppelgangerRouting::LookupDoppelgangerRouteEntriesIP (destAddress);

  if (routeEntries.size() > 0 ) {
    NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " Entry Hit for " << stringIP(destAddress.Get()) << " Found " << routeEntries.size() << " Entries");
    //for ( std::vector<DoppelgangerRouteEntry>::iterator it = routeEntries.begin(); it != routeEntries.end(); ++it){
    //  printIP(it->network.Get());
   // }
  } else {
    NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " Entries MISS for " << stringIP(destAddress.Get()));
    return false;
  }

  //Ecmp
  uint32_t selectedPort = routeEntries[rand() % routeEntries.size ()].port;


  NS_LOG_WARN("Host: " << stringIP(m_addr.Get()) << " Setting up route for dest address " << stringIP(header.GetDestination().Get()) << " port " <<  selectedPort );
  Ptr<Ipv4Route> route = Ipv4DoppelgangerRouting::ConstructIpv4Route (selectedPort, destAddress);

  //ucb (route, packet, header);
  ucb (route, packet, headerPrime);

  return true;
}

void
Ipv4DoppelgangerRouting::NotifyInterfaceUp (uint32_t interface)
{
}

void
Ipv4DoppelgangerRouting::NotifyInterfaceDown (uint32_t interface)
{
}

void
Ipv4DoppelgangerRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4DoppelgangerRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4DoppelgangerRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_LOGIC (this << "Setting up Ipv4: " << ipv4);
  NS_ASSERT (m_ipv4 == 0 && ipv4 != 0);
  m_ipv4 = ipv4;
}
/*
void
Ipv4DoppelgangerRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
  printf("routing-table-stub");
}
*/

void
Ipv4DoppelgangerRouting::PrintRoutingTable (Ptr< OutputStreamWrapper > stream, Time::Unit unit) const
{
}


void
Ipv4DoppelgangerRouting::DoDispose (void)
{
  std::map<uint32_t, Flowlet *>::iterator itr = m_flowletTable.begin ();
  for ( ; itr != m_flowletTable.end (); ++itr )
  {
    delete (itr->second);
  }
  m_dreEvent.Cancel ();
  m_agingEvent.Cancel ();
  m_ipv4=0;
  Ipv4RoutingProtocol::DoDispose ();
}

uint32_t
Ipv4DoppelgangerRouting::UpdateLocalDre (const Ipv4Header &header, Ptr<Packet> packet, uint32_t port)
{
  uint32_t X = 0;
  std::map<uint32_t, uint32_t>::iterator XItr = m_XMap.find(port);
  if (XItr != m_XMap.end ())
  {
    X = XItr->second;
  }
  uint32_t newX = X + packet->GetSize () + header.GetSerializedSize ();
  NS_LOG_LOGIC (this << " Update local dre, new X: " << newX);
  m_XMap[port] = newX;
  return newX;
}

void
Ipv4DoppelgangerRouting::DreEvent ()
{
  bool moveToIdleStatus = true;

  std::map<uint32_t, uint32_t>::iterator itr = m_XMap.begin ();
  for ( ; itr != m_XMap.end (); ++itr )
  {
    uint32_t newX = itr->second * (1 - m_alpha);
    itr->second = newX;
    if (newX != 0)
    {
      moveToIdleStatus = false;
    }
  }

  NS_LOG_LOGIC (this << " Dre event finished, the dre table is now: ");
  Ipv4DoppelgangerRouting::PrintDreTable ();

  if (!moveToIdleStatus)
  {
    m_dreEvent = Simulator::Schedule(m_tdre, &Ipv4DoppelgangerRouting::DreEvent, this);
  }
  else
  {
    NS_LOG_LOGIC (this << " Dre event goes into idle status");
  }
}

void
Ipv4DoppelgangerRouting::AgingEvent ()
{
    bool moveToIdleStatus = true;
    std::map<uint32_t, std::map<uint32_t, std::pair<Time, uint32_t> > >::iterator itr =
        m_doppelgangerToLeafTable.begin ();
    for ( ; itr != m_doppelgangerToLeafTable.end (); ++itr)
    {
      std::map<uint32_t, std::pair<Time, uint32_t> >::iterator innerItr =
        (itr->second).begin ();
      for (; innerItr != (itr->second).end (); ++innerItr)
      {
        if (Simulator::Now () - (innerItr->second).first > m_agingTime)
        {
          (innerItr->second).second = 0;
        }
        else
        {
          moveToIdleStatus = false;
        }
      }
    }
    std::map<uint32_t, std::map<uint32_t, FeedbackInfo> >::iterator itr2 =
        m_doppelgangerFromLeafTable.begin ();
    for ( ; itr2 != m_doppelgangerFromLeafTable.end (); ++itr2 )
    {
        std::map<uint32_t, FeedbackInfo>::iterator innerItr2 =
          (itr2->second).begin ();
        for ( ; innerItr2 != (itr2->second).end (); ++innerItr2)
        {
          if (Simulator::Now () - (innerItr2->second).updateTime > m_agingTime)
          {
            (itr2->second).erase (innerItr2);
            if ((itr2->second).empty ())
            {
                m_doppelgangerFromLeafTable.erase (itr2);
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
      NS_LOG_LOGIC (this << " Aging event goes into idle status");
    }
}

uint32_t
Ipv4DoppelgangerRouting::QuantizingX (uint32_t interface, uint32_t X)
{
  DataRate c = m_C;
  std::map<uint32_t, DataRate>::iterator itr = m_Cs.find (interface);
  if (itr != m_Cs.end ())
  {
    c = itr->second;
  }
  double ratio = static_cast<double> (X * 8) / (c.GetBitRate () * m_tdre.GetSeconds () / m_alpha);
  NS_LOG_LOGIC ("ratio: " << ratio);
  return static_cast<uint32_t>(ratio * std::pow(2, m_Q));
}

void
Ipv4DoppelgangerRouting::PrintDoppelgangerToLeafTable ()
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

void
Ipv4DoppelgangerRouting::PrintDoppelgangerFromLeafTable ()
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

void
Ipv4DoppelgangerRouting::PrintFlowletTable ()
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

void
Ipv4DoppelgangerRouting::PrintDreTable ()
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


}

