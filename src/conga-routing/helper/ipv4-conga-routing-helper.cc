/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/ipv4-conga-routing-helper.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4CongaRoutingHelper");

Ipv4CongaRoutingHelper::Ipv4CongaRoutingHelper ()
{

}

Ipv4CongaRoutingHelper::Ipv4CongaRoutingHelper (const Ipv4CongaRoutingHelper&)
{

}
  
Ipv4CongaRoutingHelper* 
Ipv4CongaRoutingHelper::Copy (void) const
{
  printf("calling the conga routing helper!!\n");
  return new Ipv4CongaRoutingHelper (*this); 
}
  
Ptr<Ipv4RoutingProtocol>
Ipv4CongaRoutingHelper::Create (Ptr<Node> node) const
{
  printf("Calling the conga ipv4 creator method\n");
  Ptr<Ipv4CongaRouting> congaRouting = CreateObject<Ipv4CongaRouting> ();
  return congaRouting;
}



Ptr<Ipv4CongaRouting> 
Ipv4CongaRoutingHelper::GetCongaRouting (Ptr<Ipv4> ipv4) const
{
  Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol ();
  if (DynamicCast<Ipv4CongaRouting> (ipv4rp))
  {
    NS_LOG_WARN ("Ipv4CongaRouting found as the main IPv4 routing protocol");
    return DynamicCast<Ipv4CongaRouting> (ipv4rp); 
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

