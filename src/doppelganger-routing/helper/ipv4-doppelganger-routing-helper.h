/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_DOPPELGANGER_ROUTING_HELPER_H
#define IPV4_DOPPELGANGER_ROUTING_HELPER_H

#include "ns3/ipv4-doppelganger-routing.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class Ipv4DoppelgangerRoutingHelper : public Ipv4RoutingHelper
{
public:
  Ipv4DoppelgangerRoutingHelper ();
  Ipv4DoppelgangerRoutingHelper (const Ipv4DoppelgangerRoutingHelper&);
  
  Ipv4DoppelgangerRoutingHelper* Copy (void) const;
  
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

  Ptr<Ipv4DoppelgangerRouting> GetDoppelgangerRouting (Ptr<Ipv4> ipv4) const;
};

}

#endif /* IPV4_DOPPELGANGER_ROUTING_HELPER_H */

