/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <algorithm>

#include "rpc-server.h"
#include "ns3/ipv4-doppelganger-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RpcServerApplication");

NS_OBJECT_ENSURE_REGISTERED (RpcServer);

uint64_t 
ServerLoadAtTime(uint server_id, uint64_t time, std::vector<std::vector<LoadEvent>> *global_load_log) {
  uint64_t last_event_time=0;
  int64_t load = 0;
  NS_LOG_WARN("Searching for load on server " << server_id);
  for(uint i=0;i<(*global_load_log)[server_id].size();i++) {
    LoadEvent le = (*global_load_log)[server_id][i];
    //Break the loop if we have seen all events to this point in time
    if (le.time > time) {
      break;
    }
    //subtract load for gap in time between last events
    load -= (le.time - last_event_time);
    if (load < 0) {
      load = 0;
    }
    // add load for new event
    load += le.load;
    last_event_time = le.time;
  }
  //subtract for the final gap between the last event and the current time
  load -= Simulator::Now().GetNanoSeconds() - last_event_time;
  if (load < 0) {
    load = 0;
  }
  return (uint64_t) load;
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

TypeId
RpcServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RpcServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<RpcServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&RpcServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

RpcServer::RpcServer ()
{
  NS_LOG_FUNCTION (this);
}

RpcServer::~RpcServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
RpcServer::AddRpc(int rpc_id) {
  m_serviceable_rpcs.push_back(rpc_id);
}

bool
RpcServer::CanServiceRPC(int rpc_id) {
  if (std::find(m_serviceable_rpcs.begin(), m_serviceable_rpcs.end(), rpc_id) != m_serviceable_rpcs.end())
		return true;
	else
    return false;
}

void RpcServer::AssignRPC(std::vector<int> services) {
  for (uint i =0; i< services.size(); i++) {
    AddRpc(services[i]);
  }
}

void RpcServer::SetGlobalLoad(uint64_t * serverLoad) {
  m_serverLoad = serverLoad;
}

void RpcServer::SetGlobalLoadUpdate(Time * serverLoad_update) {
  m_serverLoad_update = serverLoad_update;
}

void RpcServer::SetID(int id) {
  m_id = id;
}

void
RpcServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
RpcServer::SetLoadDistribution(std::vector<uint32_t> loadDistribution){
  m_load_distribution = loadDistribution;
}

std::vector<uint32_t> 
RpcServer::GetLoadDistribution(){
  return m_load_distribution;
}

void
RpcServer::SetGlobalLoadLog(std::vector<std::vector<LoadEvent>> *global_load_log) {
  m_load_log = global_load_log;
}


uint32_t 
RpcServer::GetRequestLoad(){
  return m_load_distribution[rand() % m_load_distribution.size()];
}


void 
RpcServer::SetClientAddresses(uint32_t num_clients, uint16_t port, Address * addresses) {
  for (uint i=0;i<num_clients;i++) {
    m_client_addresses.push_back(addresses[i]);
    m_client_ports.push_back(port);
  }
}

uint32_t
RpcServer::GetInstantenousLoad() {
  Time now = Simulator::Now();
  int64_t time_passed;
  time_passed = now.GetNanoSeconds() - m_serverLoad_update[m_id].GetNanoSeconds();

  //Recalculate load
  int64_t tmp_load = m_serverLoad[m_id]; //use a tmp variable to prevent overflow
  tmp_load -= time_passed;

  //Prevent load from going below 0
  if (tmp_load < 0) {
    tmp_load = 0;
  }
  //Update Server Load
  m_serverLoad[m_id] = tmp_load;
  m_serverLoad_update[m_id] = now;

  return m_serverLoad[m_id];
}

void 
RpcServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      if (m_socket6->Bind (local6) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&RpcServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&RpcServer::HandleRead, this));

  //If periodic information spread is turned on start the loopping process
  if(m_information_function == periodic) {
    SpreadLoadInformation();
  }

}

void 
RpcServer::SpreadLoadInformation() {
  NS_LOG_INFO("Spreading information on host " <<  m_id << " To all clients at time " << Simulator::Now().GetNanoSeconds());
  //create a packet with Load informatyion
  //Aquire IP's of every client
  //spread the information to every client
  Simulator::Schedule(NanoSeconds(m_information_spread_period), &RpcServer::SpreadLoadInformation, this);

}

void 
RpcServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
RpcServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if(m_id == 666) {
        NS_LOG_WARN("MIDDDLE BOX!!\n");
      }
	      if (InetSocketAddress::IsMatchingType (from))
		{
		  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
			       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
			       InetSocketAddress::ConvertFrom (from).GetPort ());
		}
	      else
		{
		  NS_LOG_INFO ("UNSUPPORTED PROTOCOL IPV6");
		}

      //packet->RemoveAllPacketTags ();
      //packet->RemoveAllByteTags ();

      //Check if the service asked for by the client is available on the server

      Ipv4DoppelgangerTag doppelTag;
      bool found = packet->PeekPacketTag(doppelTag);

      if (found) {
        NS_LOG_WARN("Found the packet tag on the end server");
      } else {
        NS_LOG_WARN("Packet arrived but it does not have the correct tag");
      }


      for (int i =0; i < MAX_REPLICAS; i++) {
        NS_LOG_INFO("Service also servicable by " << stringIP(doppelTag.GetReplica(i)));
      }

      if (CanServiceRPC(doppelTag.GetRequestID())) {
        NS_LOG_INFO("Able to service request id " << doppelTag.GetRequestID() << " on server " << m_id << " for packet sent from  " << InetSocketAddress::ConvertFrom (from).GetIpv4 ());
      } else {
        NS_LOG_INFO("Unable to service request id " << doppelTag.GetRequestID() << " on server " << m_id << " for packet sent from  " << InetSocketAddress::ConvertFrom (from).GetIpv4 ());
      }

      //Calculate server load
      int requestProcessingTime = GetRequestLoad();
      int load = GetInstantenousLoad();
      // Set the current load of the server 
      (m_serverLoad)[m_id] = load + requestProcessingTime; //make a distribution in the future

      //Update the Event Log
      LoadEvent le = {(uint64_t)Simulator::Now().GetNanoSeconds(),(uint64_t)requestProcessingTime};
      (*m_load_log)[m_id].push_back(le);


      //Remove the replica tags for the server, the client has only one response location
      packet->RemovePacketTag(doppelTag);
      //TODO add the rest of the packet fields for the return trip
      for (int i =0; i < MAX_REPLICAS; i++) {
        doppelTag.SetReplica(i,InetSocketAddress::ConvertFrom (from).GetIpv4().Get());
        //printf("%d\n",InetSocketAddress::ConvertFrom (from).GetIpv4().Get());
      }
      doppelTag.SetCanRouteDown(true); //There should be no limitations on how the packet is routed back to the client
      
      //Piggyback information if piggybacking is set.
      if (m_information_function == piggyback) {
        doppelTag.SetPacketType(Ipv4DoppelgangerTag::response_piggyback);
        doppelTag.SetHostSojournTime(m_serverLoad[m_id]);
      } else {
        doppelTag.SetPacketType(Ipv4DoppelgangerTag::response);
      }
      //printf("Tag 0:%d\n",doppelTag.GetReplica(0));
      packet->AddPacketTag(doppelTag);
      ScheduleResponse(NanoSeconds(((m_serverLoad)[m_id])), socket, packet, from, requestProcessingTime);

      //Inject some periodic delay.
      if (m_last_delay == 0 || (Simulator::Now().GetNanoSeconds() - m_last_delay) > m_min_time_between_delays) {
        if (rand() % 500) {
          (m_serverLoad)[m_id] = GetInstantenousLoad() + m_delay_time;
           m_last_delay = Simulator::Now().GetNanoSeconds();
        }
      }

    }
}


void RpcServer::ScheduleResponse(Time dt,Ptr<Socket> socket, Ptr<Packet> packet, Address from, int load)
{
  NS_LOG_FUNCTION(this << dt);
  Simulator::Schedule(dt, &RpcServer::SendResponse, this, socket, packet, from, load);
}

void
RpcServer::SendResponse(Ptr<Socket> socket, Ptr<Packet> packet, Address from,int load) {
      NS_LOG_LOGIC ("Responding with original Packet");

      //Send the current load back along the reply path
      Ipv4DoppelgangerTag doppelTag;
      bool found = packet->PeekPacketTag(doppelTag);
      if (found) {
        NS_LOG_WARN("Found the packet tag on the end server");
      } else {
        NS_LOG_WARN("Packet arrived but it does not have the correct tag");
      }
      int current_load = GetInstantenousLoad();
      packet->RemovePacketTag(doppelTag);
      doppelTag.SetHostLoad(current_load);
      packet->AddPacketTag(doppelTag);

      socket->SendTo (packet, 0, from);

	      if (InetSocketAddress::IsMatchingType (from))
		{
		  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
			       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
			       InetSocketAddress::ConvertFrom (from).GetPort ());
		}
	      else
		{
		  NS_LOG_INFO ("DOING NOTHING UNSUPPORTED PROTOCL");
		}
}

} // Namespace ns3
