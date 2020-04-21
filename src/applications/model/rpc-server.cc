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
#include "rpc.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RpcServerApplication");

NS_OBJECT_ENSURE_REGISTERED (RpcServer);

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

void RpcServer::SetGlobalLoad(uint64_t ** serverLoad) {
  m_serverLoad = serverLoad;
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
      RPCHeader rpch;
      packet->CopyData((uint8_t*)&rpch,sizeof(RPCHeader));

      for (int i =0; i < MAX_REPLICAS; i++) {
        NS_LOG_INFO("Service also servicable by " << rpch.Replicas[i]);
      }

      if (CanServiceRPC(rpch.RequestID)) {
        NS_LOG_INFO("Able to service request id " << rpch.RequestID << " on server " << m_id << " for packet sent from  " << InetSocketAddress::ConvertFrom (from).GetIpv4 ());
      } else {
        NS_LOG_INFO("Unable to service request id " << rpch.RequestID << " on server " << m_id << " for packet sent from  " << InetSocketAddress::ConvertFrom (from).GetIpv4 ());
      }

      //Calculate server load
      int load = 10;
      (*m_serverLoad)[m_id] += load; //make a distribution in the future
      ScheduleResponse(MicroSeconds(((*m_serverLoad)[m_id])), socket, packet, from, load);

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
      (*m_serverLoad)[m_id] -= load;
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
