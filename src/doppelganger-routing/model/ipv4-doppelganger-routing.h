/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_DOPPELGANGER_ROUTING_H
#define IPV4_DOPPELGANGER_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ipv4-doppelganger-tag.h"

#include <map>
#include <vector>

namespace ns3 {

struct Flowlet {
  uint32_t port;
  Time activeTime;
};

struct FeedbackInfo {
  uint32_t ce;
  bool change;
  Time updateTime;
};

struct DoppelgangerRouteEntry {
  Ipv4Address network;
  Ipv4Mask networkMask;
  uint32_t port;
};



class Ipv4DoppelgangerRouting : public Ipv4RoutingProtocol
{
public:

  enum LoadBalencingStrategy
  {
    none = 0,
    minimumLoad = 1,
    coreOnly = 2,
  };

  enum FatTreeSwitchType
  {
    endhost = 0,
    edge = 1,
    agg = 2,
    core = 3,
  };

  std::string FatTreeTypeToString(FatTreeSwitchType ftst) {
    switch(ftst){
      case endhost:
        return "endhost";
      case edge:
        return "edge";
        break;
      case agg:
        return "agg";
        break;
      case core:
        return "core";
        break;
      default:
        return "Unknown Switch Type";
    }
    return "Unknown and Unreachable Switch type (error here)";
  }

  //Conga functions : TODO remove
  Ipv4DoppelgangerRouting ();
  ~Ipv4DoppelgangerRouting ();
  static TypeId GetTypeId (void);
  void SetLeafId (uint32_t leafId);
  void SetAlpha (double alpha);
  void SetTDre (Time time);
  void SetLinkCapacity (DataRate dataRate);
  void SetLinkCapacity (uint32_t interface, DataRate dataRate);
  void SetQ (uint32_t q);
  void SetFlowletTimeout (Time timeout);
  void AddAddressToLeafIdMap (Ipv4Address addr, uint32_t leafId);
  void AddRoute (Ipv4Address network, Ipv4Mask networkMask, uint32_t port);
  void InitCongestion (uint32_t destLeafId, uint32_t port, uint32_t congestion);
  void EnableEcmpMode ();

  //Doppelganger routing
  void SetRpcServices(std::vector<std::vector<int>> rpcServices);
  void SetIPServerMap(std::map<uint32_t,uint32_t> ip_map);
  void SetGlobalServerLoad(uint64_t *serverLoad);
  void SetGlobalServerLoadUpdate(Time *serverLoad_update);
  void SetLoadBalencingStrategy(LoadBalencingStrategy strategy);

  void SetFatTreeSwitchType(FatTreeSwitchType ftst);
  FatTreeSwitchType GetFatTreeSwitchType();

  uint64_t GetPacketRedirections();
  uint64_t GetTotalPackets();

  void SetAddress(Ipv4Address addr);
  Ipv4Address GetAddress();


  uint64_t GetInstantenousLoad(int server_id);
  uint32_t replicaSelectionStrategy_minimumLoad(uint32_t ips[MAX_REPLICAS]);


  /* Inherit From Ipv4RoutingProtocol */
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
 // virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;
  virtual void PrintRoutingTable (Ptr< OutputStreamWrapper > stream, Time::Unit unit=Time::S) const;

  virtual void DoDispose (void);

private:

  // CONGA Routing Variables, TODO remove

  // Used to determine whether this switch is leaf switch
  bool m_isLeaf;
  uint32_t m_leafId;
  // DRE algorithm related parameters
  Time m_tdre;
  double m_alpha;
  // Link capacity used to quantizing X
  DataRate m_C;
  std::map<uint32_t, DataRate> m_Cs;
  // Quantizing bits
  uint32_t m_Q;
  Time m_agingTime;
  // Flowlet Timeout
  Time m_flowletTimeout;
  // Dev use
  bool m_ecmpMode;
  // ------ Variables ------
  // Used to maintain the round robin
  unsigned long m_feedbackIndex;
  // Dre Event ID
  EventId m_dreEvent;
  // Metric aging event
  EventId m_agingEvent;
  // Ipv4 associated with this router
  Ptr<Ipv4> m_ipv4;
  // Route table
  std::vector<DoppelgangerRouteEntry> m_routeEntryList;
  // Ip and leaf switch map,
  // used to determine the which leaf switch the packet would go through
  std::map<Ipv4Address, uint32_t> m_ipLeafIdMap;
  // Congestion To Leaf Table
  std::map<uint32_t, std::map<uint32_t, std::pair<Time, uint32_t> > > m_doppelgangerToLeafTable;
  // Congestion From Leaf Table
  std::map<uint32_t, std::map<uint32_t, FeedbackInfo> > m_doppelgangerFromLeafTable;
  // Flowlet Table
  std::map<uint32_t, Flowlet *> m_flowletTable;
  // Parameters
  // DRE
  std::map<uint32_t, uint32_t> m_XMap;


  Ipv4Address m_addr;
  // global server load
  uint64_t *m_serverLoad;                             
  Time *m_serverLoad_update;                             
  //replica locations
  std::vector<std::vector<int>> m_rpc_server_replicas;
  //Ip to replica index
  std::map<uint32_t,uint32_t> m_server_ip_map;
  //in network load ballencing strategy
  LoadBalencingStrategy m_load_balencing_strategy;
  // Location in a fat tree network
  FatTreeSwitchType m_fattree_switch_type;
  //A count of the number of times a specific router has redirected packets to a different host
  uint64_t m_packet_redirections;
  uint64_t m_total_packets;




  // ------ Functions ------
  // DRE algorithm
  uint32_t UpdateLocalDre (const Ipv4Header &header, Ptr<Packet> packet, uint32_t path);
  void DreEvent();
  void AgingEvent ();

  // Quantizing X to metrics degree
  // X is bytes here and we quantizing it to 0 - 2^Q
  uint32_t QuantizingX (uint32_t interface, uint32_t X);
  std::vector<DoppelgangerRouteEntry> LookupDoppelgangerRouteEntries (Ipv4Address dest);
  std::vector<DoppelgangerRouteEntry> LookupDoppelgangerRouteEntriesIP (Ipv4Address dest);
  Ptr<Ipv4Route> ConstructIpv4Route (uint32_t port, Ipv4Address destAddress);

  // Debug use
  void PrintDoppelgangerToLeafTable ();
  void PrintDoppelgangerFromLeafTable ();
  void PrintFlowletTable ();
  void PrintDreTable ();

};

}

#endif /* DOPPELGANGER_ROUTING_H */

