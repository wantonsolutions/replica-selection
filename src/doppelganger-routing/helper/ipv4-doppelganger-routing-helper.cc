/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/ipv4-doppelganger-routing-helper.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4DoppelgangerRoutingHelper");

Ipv4DoppelgangerRoutingHelper::Ipv4DoppelgangerRoutingHelper ()
{

}

Ipv4DoppelgangerRoutingHelper::Ipv4DoppelgangerRoutingHelper (const Ipv4DoppelgangerRoutingHelper&)
{

}
  
Ipv4DoppelgangerRoutingHelper* 
Ipv4DoppelgangerRoutingHelper::Copy (void) const
{
  return new Ipv4DoppelgangerRoutingHelper (*this); 
}
  
Ptr<Ipv4RoutingProtocol>
Ipv4DoppelgangerRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<Ipv4DoppelgangerRouting> doppelgangerRouting = CreateObject<Ipv4DoppelgangerRouting> ();
  return doppelgangerRouting;
}



Ptr<Ipv4DoppelgangerRouting> 
Ipv4DoppelgangerRoutingHelper::GetDoppelgangerRouting (Ptr<Ipv4> ipv4) const
{
  Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol ();
  if (DynamicCast<Ipv4DoppelgangerRouting> (ipv4rp))
  {
    return DynamicCast<Ipv4DoppelgangerRouting> (ipv4rp); 
  }
  return 0;
}

}

