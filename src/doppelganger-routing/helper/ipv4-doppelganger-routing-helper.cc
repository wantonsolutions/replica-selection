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
  printf("calling the doppelganger routing helper!!\n");
  return new Ipv4DoppelgangerRoutingHelper (*this); 
}
  
Ptr<Ipv4RoutingProtocol>
Ipv4DoppelgangerRoutingHelper::Create (Ptr<Node> node) const
{
  printf("Calling the doppelganger ipv4 creator method\n");
  Ptr<Ipv4DoppelgangerRouting> doppelgangerRouting = CreateObject<Ipv4DoppelgangerRouting> ();
  return doppelgangerRouting;
}



Ptr<Ipv4DoppelgangerRouting> 
Ipv4DoppelgangerRoutingHelper::GetDoppelgangerRouting (Ptr<Ipv4> ipv4) const
{
  Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol ();
  if (DynamicCast<Ipv4DoppelgangerRouting> (ipv4rp))
  {
    NS_LOG_WARN ("Ipv4DoppelgangerRouting found as the main IPv4 routing protocol");
    return DynamicCast<Ipv4DoppelgangerRouting> (ipv4rp); 
  }
  return 0;
}
/*
Ptr<Ipv4RoutingProtocol>
Ipv4GlobalRoutingHelper::Create (Ptr<Node> node) const
{
  NS_LOG_LOGIC ("Adding GlobalRouter interface to node " <<
                node->GetId ());

  Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter> ();
  node->AggregateObject (globalRouter);

  NS_LOG_LOGIC ("Adding GlobalRouting Protocol to node " << node->GetId ());
  Ptr<Ipv4GlobalRouting> globalRouting = CreateObject<Ipv4GlobalRouting> ();
  globalRouter->SetRoutingProtocol (globalRouting);

  return globalRouting;
}*/



}

