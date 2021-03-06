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

#include "ns3/ipv4-packet-info-tag.h"

#include "raid-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RaidServerApplication");

NS_OBJECT_ENSURE_REGISTERED (RaidServer);

//int min, max;

TypeId
RaidServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RaidServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<RaidServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&RaidServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Parallel", "The degree to which the underlying parallel fat tree is parallelized.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&RaidServer::m_parallel),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}


RaidServer::RaidServer ()
{
  NS_LOG_FUNCTION (this);
}

RaidServer::~RaidServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  //Min and Max refer to the minumum and maximum request values received. These are yet to be implemented but will act as a high water mark in the future.
  //TODO uncomment
  //min = 0;
  //max = 0;
  //none of the requests have been served at init time, set each to false.
}

void
RaidServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
RaidServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  //Init raid state data
  m_rs = InitRaidState(m_parallel); 
  //Initalize parallel sockets
  m_sockets = new Ptr<Socket>[m_parallel];
  for (int i=0;i<m_parallel;i++) {
	//connect a seperate socket to each of the net devices attacehd to the server node
	Ptr<Node> n = GetNode();
	Ptr<NetDevice> dev = n->GetDevice(i);
	m_sockets[i] = ConnectSocket(m_port,dev);
	m_sockets[i]->SetRecvCallback (MakeCallback (&RaidServer::HandleRead, this));
	m_sockets[i]->SetAllowBroadcast (true);
	//TODO remove if a single socket does not cut it
	break;
	//This may still be usefull, buf for the moment only a single socket is used while listening because all of the client traffic is forwarded to it.


  }

}

Ptr<Socket>
RaidServer::ConnectSocket(uint16_t port, Ptr<NetDevice> dev) {
  Ptr<Socket> socket;
  if (socket == 0)
    {
      NS_LOG_INFO("Setting up server sockets\n");
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      socket = Socket::CreateSocket (GetNode (), tid);

      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
      if (socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }
    return socket;
}

void 
RaidServer::StopApplication ()
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
RaidServer::BroadcastWrite(Ptr<Packet> packet, Ptr<Socket> socket, Address from) {
	//This function calculates the IP's that the packet was not received on, and
	//broadcasts the packet back over those channels.
	InetSocketAddress addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
	NS_LOG_INFO( "Address value " << addr.GetIpv4().Get() );

	//The third unit of the IP address is used for paralleization i.e.
	//X.Y.X.X, the Y digit will be used to broadcast in parallel. This
	//function determines which of the parallel chanels the packet was
	//received on and then broadcasts across the rest.
	
	int mask = 0x00FF0000;
	int invmask = 0xFF00FFFF;
	int hitIndex = ((addr.GetIpv4().Get() & mask) >> 16);
	NS_LOG_INFO( "Addr Key " << hitIndex);
	//TODO there is probably a cleaner way to do this by just casting the from address rather than re-initalizing
	for (int i =1; i <= m_parallel; i++) {
		int newAddr32 = (addr.GetIpv4().Get() & invmask) + (i << 16);
		Ipv4Address tmpAddr = addr.GetIpv4();
		tmpAddr.Set(newAddr32);
		addr.SetIpv4(tmpAddr);
		addr.SetPort(InetSocketAddress::ConvertFrom (from).GetPort ());
		socket->SendTo (packet, 0, addr);
		VerboseServerSendPrint(addr,packet);
	}
}





void 
RaidServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {

       VerboseServerReceivePrint(from,packet);

      //packet->RemoveAllPacketTags ();
      //packet->RemoveAllByteTags ();
       
       Ptr<Packet> p = RaidReceive(packet, from, m_rs, m_parallel);
       if ( p == NULL ) {
	       continue;
       }

       Ipv4PacketInfoTag idtag;
       p->PeekPacketTag(idtag);
       //TODO maintain high and low watermark with at seperate tag
       int requestIndex = idtag.GetRecvIf();
       uint8_t *data = new uint8_t[p->GetSize()];
       p->CopyData(data,p->GetSize());
       Ptr<Packet> *rpackets = StripePacket(m_parallel, p->GetSize(),requestIndex, data);
       RaidWrite(requestIndex,m_rs,rpackets,socket,from,m_parallel);


	InetSocketAddress addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
	int invmask = 0xFF00FFFF;
	for (int i =1; i <= m_parallel; i++) {
		int newAddr32 = (addr.GetIpv4().Get() & invmask) + (i << 16);
		Ipv4Address tmpAddr = addr.GetIpv4();
		tmpAddr.Set(newAddr32);
		addr.SetIpv4(tmpAddr);
		addr.SetPort(InetSocketAddress::ConvertFrom (from).GetPort ());
		VerboseServerSendPrint(addr,rpackets[i-1]);
	}


       

   }
}

void RaidServer::VerboseServerReceivePrint(Address from, Ptr<Packet> packet) {
      if (InetSocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
		       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
		       InetSocketAddress::ConvertFrom (from).GetPort ());
	}
      else if (Inet6SocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
		       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
		       Inet6SocketAddress::ConvertFrom (from).GetPort ());
	}

}

void RaidServer::VerboseServerSendPrint(Address from, Ptr<Packet> packet) {
      if (InetSocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
		       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
		       InetSocketAddress::ConvertFrom (from).GetPort ());
	}
      else if (Inet6SocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
		       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
		       Inet6SocketAddress::ConvertFrom (from).GetPort ());
	}
}

} // Namespace ns3
