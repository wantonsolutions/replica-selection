
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
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "rpc-client.h"
#include "rpc.h"

//#include "ns3/lte-pdcp-tag.h"
#include "ns3/ipv4-packet-info-tag.h"
#include <iostream>
#include <fstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RpcClientApplication");

NS_OBJECT_ENSURE_REGISTERED(RpcClient);

TypeId
RpcClient::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::RpcClient")
                          .SetParent<Application>()
                          .SetGroupName("Applications")
                          .AddConstructor<RpcClient>()
                          .AddAttribute("MaxPackets",
                                        "The maximum number of packets the application will send",
                                        UintegerValue(100),
                                        MakeUintegerAccessor(&RpcClient::m_count),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Interval",
                                        "The time to wait between packets",
                                        TimeValue(Seconds(1.0)),
                                        MakeTimeAccessor(&RpcClient::m_interval),
                                        MakeTimeChecker())
                          .AddAttribute("RemoteAddress",
                                        "The destination Address of the outbound packets",
                                        AddressValue(),
                                        MakeAddressAccessor(&RpcClient::m_peerAddress),
                                        MakeAddressChecker())
                          .AddAttribute("RemotePort",
                                        "The destination port of the outbound packets",
                                        UintegerValue(0),
                                        MakeUintegerAccessor(&RpcClient::m_peerPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("PacketSize", "Size of echo data in outbound packets",
                                        UintegerValue(100),
                                        MakeUintegerAccessor(&RpcClient::SetDataSize,
                                                             &RpcClient::GetDataSize),
                                        MakeUintegerChecker<uint32_t>())
                          .AddTraceSource("Tx", "A new packet is created and is sent",
                                          MakeTraceSourceAccessor(&RpcClient::m_txTrace),
                                          "ns3::Packet::TracedCallback");
  return tid;
}

RpcClient::RpcClient()
{
  NS_LOG_FUNCTION(this);
  m_sent = 0;
  m_rec = 0;
  m_socket = 0;
  m_intervalRatio = 0.01;
  m_sendEvent = EventId();
  m_data = 0;
  m_dataSize = 0;
}

RpcClient::~RpcClient()
{
  NS_LOG_FUNCTION(this);
  m_socket = 0;

  delete[] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void RpcClient::SetRemote(Address ip, uint16_t port)
{
  NS_LOG_FUNCTION(this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void RpcClient::SetRemote(Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_peerAddress = addr;
}

void RpcClient::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void RpcClient::StartApplication(void)
{
  NS_LOG_FUNCTION(this);

  //TODO Start here, the first thing you need to do is split on the port
  //number. Then you are going to need to go into the addressing code, and set
  //basenumbers for each of the parallel channels. Part 1 is hard Part 2 NSM.
  //Then the tricky part, on the fly design cover traffic patterns to disrupt
  //the current traffic. The most basic would be uniform traffic, Then Zipf,
  //finally bursty. There is no way you make all of that and take measurements to prepare for failure.

  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket(GetNode(), tid);
    if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
    {
      if (m_socket->Bind() == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
    else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
    {
      if (m_socket->Bind6() == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Connect(Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
    else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true)
    {
      if (m_socket->Bind() == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Connect(m_peerAddress);
    }
    else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true)
    {
      if (m_socket->Bind6() == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Connect(m_peerAddress);
    }
    else
    {
      NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
    }
  }

  m_socket->SetRecvCallback(MakeCallback(&RpcClient::HandleRead, this));
  m_socket->SetAllowBroadcast(true);
  ScheduleTransmit(Seconds(0.));
}

void RpcClient::StopApplication()
{
  NS_LOG_FUNCTION(this);

  if (m_socket != 0)
  {
    m_socket->Close();
    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_socket = 0;
  }

  Simulator::Cancel(m_sendEvent);
}

void RpcClient::SetDataSize(uint32_t dataSize)
{
  NS_LOG_FUNCTION(this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so
  // neither will we.
  //
  delete[] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t
RpcClient::GetDataSize(void) const
{
  NS_LOG_FUNCTION(this);
  return m_size;
}

void RpcClient::SetFill(std::string fill)
{
  NS_LOG_FUNCTION(this << fill);

  uint32_t dataSize = fill.size() + 1;

  if (dataSize != m_dataSize)
  {
    delete[] m_data;
    m_data = new uint8_t[dataSize];
    m_dataSize = dataSize;
  }

  memcpy(m_data, fill.c_str(), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void RpcClient::SetFill(uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION(this << fill << dataSize);
  if (dataSize != m_dataSize)
  {
    delete[] m_data;
    m_data = new uint8_t[dataSize];
    m_dataSize = dataSize;
  }

  memset(m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void RpcClient::SetFill(uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION(this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
  {
    delete[] m_data;
    m_data = new uint8_t[dataSize];
    m_dataSize = dataSize;
  }

  if (fillSize >= dataSize)
  {
    memcpy(m_data, fill, dataSize);
    m_size = dataSize;
    return;
  }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
  {
    memcpy(&m_data[filled], fill, fillSize);
    filled += fillSize;
  }

  //
  // Last fill may be partial
  //
  memcpy(&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void RpcClient::ScheduleTransmit(Time dt)
{
  NS_LOG_FUNCTION(this << dt);
  m_sendEvent = Simulator::Schedule(dt, &RpcClient::Send, this);
}


void RpcClient::SetParallel(bool parallel) {
  m_parallel = parallel;
}


void RpcClient::SetAllAddressesParallel(Address **addresses, uint16_t **ports, int **trafficMatrix, uint8_t parallel, uint32_t numPeers)
{
  m_peerAddresses_parallel = new Address *[numPeers];
  m_peerPorts_parallel = new uint16_t *[numPeers];
  m_tm = new int *[numPeers];
  m_numPeers = numPeers;
  m_parallel = true;

  for (uint32_t i = 0; i < numPeers; i++)
  {
    m_peerAddresses_parallel[i] = new Address[parallel];
    m_peerPorts_parallel[i] = new uint16_t[parallel];
    m_tm[i] = new int[numPeers];
    for (uint8_t j = 0; j < parallel; j++)
    {
      //printf("(%d,%d) portnum %d \n",i,j,ports[i][j]);
      m_peerAddresses_parallel[i][j] = addresses[i][j];
      //printf("(%d,%d)\n",i,j);
      m_peerPorts_parallel[i][j] = ports[i][j];
    }
    for (uint32_t j = 0; j < numPeers; j++)
    {
      m_tm[i][j] = trafficMatrix[i][j];
    }
  }
  return;
}

void RpcClient::SetAllAddresses(Address *addresses, uint16_t *ports, int **trafficMatrix, uint32_t numPeers)
{
  m_peerAddresses = new Address[numPeers];
  m_peerPorts = new uint16_t[numPeers];
  m_tm = new int *[numPeers];
  m_numPeers = numPeers;
  m_parallel = false;

  for (uint32_t i = 0; i < numPeers; i++)
  {
    m_peerAddresses[i] = addresses[i];
    m_peerPorts[i] = ports[i];
    m_tm[i] = new int[numPeers];
    for (uint32_t j = 0; j < numPeers; j++)
    {
      //TODO figure out what the hell traffic matrix was ever supposed to do
      m_tm[i][j] = trafficMatrix[i][j];
    }
  }
  return;
}

void RpcClient::SetGlobalPackets(uint32_t * global_packets) {
  m_global_sent = global_packets;
}

void RpcClient::SetRpcServices(std::vector<std::vector<int>> rpcServices) {
  m_rpc_server_replicas = rpcServices;
}

void RpcClient::SetGlobalSeverLoad(uint64_t **serverLoad) {
  m_serverLoad = serverLoad;
}

//For now all replicas live in the same location because there are no replicas
void RpcClient::PopulateReplicasNoReplicas(RPCHeader *rpch) {
  for( int i =0; i< MAX_REPLICAS; i++) {
    rpch->Replicas[i] = rpch->RequestID;
  }
}

void RpcClient::PopulateReplicasReplicas(RPCHeader *rpch) {
  int rpc_service = rpch->RequestID;
  if (m_rpc_server_replicas[rpc_service].size() > MAX_REPLICAS) {
    NS_LOG_WARN("Error too many replicas for a single RPC header packet");
  }
  for( uint i =0; i< m_rpc_server_replicas[rpc_service].size(); i++) {
    Ipv4Address addr = Ipv4Address::ConvertFrom(m_peerAddresses[m_rpc_server_replicas[rpc_service][i]]);
    rpch->Replicas[i] = addr.Get();
  }
}


void RpcClient::SetReplicationStrategy(int strategy){
  m_selection_strategy = strategy;
}

 int RpcClient::replicaSelectionStrategy_firstIndex(int rpc){
   return m_rpc_server_replicas[rpc][0];
 }
 int RpcClient::replicaSelectionStrategy_random(int rpc) {
   int randomReplica = rand()%m_rpc_server_replicas[rpc].size();
   return m_rpc_server_replicas[rpc][randomReplica];
 }

 int RpcClient::replicaSelectionStrategy_minimumLoad(int rpc){
   uint64_t minLoad = UINT64_MAX;
   uint64_t minReplica;
   for (uint i = 0; i < m_rpc_server_replicas[rpc].size();i++) {
     int replica = m_rpc_server_replicas[rpc][i];
     if (*m_serverLoad[replica] < minLoad){
       minLoad = *m_serverLoad[replica];
       minReplica = replica;
     }
   }
   return minReplica;
 }


void RpcClient::Send(void)
{
  NS_LOG_FUNCTION(this);

  NS_ASSERT(m_sendEvent.IsExpired());

  //Constuct RPC Request Packet
  RPCHeader rpch;
  bzero((char *)&rpch,sizeof(rpch));
  rpch.PacketID = 0;
  //Determine the RPC request Type for now set it to random
  rpch.RequestID = rand() % m_numPeers;
  //PopulateReplicasNoReplicas(&rpch);
  PopulateReplicasReplicas(&rpch);
  SetFill((uint8_t*)&rpch,sizeof(RPCHeader),sizeof(RPCHeader));

  Ptr<Packet> p;
  if (m_dataSize)
  {
    //
    // If m_dataSize is non-zero, we have a data buffer of the same size that we
    // are expected to copy and send.  This state of affairs is created if one of
    // the Fill functions is called.  In this case, m_size must have been set
    // to agree with m_dataSize
    //
    NS_ASSERT_MSG(m_dataSize == m_size, "RpcClient::Send(): m_size and m_dataSize inconsistent");
    NS_ASSERT_MSG(m_data, "RpcClient::Send(): m_dataSize but no m_data");
    p = Create<Packet>(m_data, m_dataSize);
  }
  else
  {
    //
    // If m_dataSize is zero, the client has indicated that it doesn't care
    // about the data itself either by specifying the data size by setting
    // the corresponding attribute or by not calling a SetFill function.  In
    // this case, we don't worry about it either.  But we do allow m_size
    // to have a value different from the (zero) m_dataSize.
    //
    p = Create<Packet>(m_size);
  }
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  //

  //PdcpTag idtag;
  Ipv4PacketInfoTag idtag;
  //idtag.SetSenderTimestamp(Time(m_sent));
  //printf("Sending Packet %d\n",m_sent);
  uint32_t send_index = m_sent % REQUEST_BUFFER_SIZE;
  idtag.SetRecvIf(send_index);
  p->AddPacketTag(idtag);
  m_requests[m_sent % REQUEST_BUFFER_SIZE] = Simulator::Now();




  //Find Replicas of the RPC


  m_txTrace(p);

  //send to a random server
  // TODO TODO TODO Start here next time choose the address to send to relient on the peer
  if (m_parallel == true) {
    m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses_parallel[(2 * rand()) % m_numPeers][rand() % 3]), m_peerPort));
  } else {
    //Send to a random node on a random part of a parallel fat tree
    //m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses[(2 * rand()) % m_numPeers]), m_peerPort));

    //Send to node 1
    //m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses[1]), m_peerPort));

    //Send to a random node assumes that all server nodes have a service running
    //m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses[rand() % m_numPeers]), m_peerPort));

    //Send to a node specified in the RPC header. In this case there is only one becasue there are no middle boxes

    int replica;
    switch (m_selection_strategy) {
      case noReplica:
        replica = replicaSelectionStrategy_firstIndex(rpch.RequestID);
        break;
      case randomReplica:
        replica = replicaSelectionStrategy_random(rpch.RequestID);
        break;
      case minimumReplica:
        replica = replicaSelectionStrategy_random(rpch.RequestID);
        break;
      default:
        NS_LOG_WARN("The selection strategy " << m_selection_strategy << " is invalid ");
    }
    //int replica = replicaSelectionStrategy_minimumLoad(rpch.RequestID);
    m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses[replica]), m_peerPort));

    //Test breakage
    //m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses[(rpch.RequestID+1) % m_numPeers]), m_peerPort));

  }

  // \send to a random server

  m_socket->Send(p);

  ++m_sent;
  ++(*m_global_sent);
  if (Ipv4Address::IsMatchingType(m_peerAddresses[rpch.RequestID]))
  {
    NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s client sent local packet "<< m_sent << " gloabal packet " << *m_global_sent << " of size " << m_size << " bytes to " << Ipv4Address::ConvertFrom(m_peerAddresses[rpch.RequestID]) << " port " << m_peerPort);
  } else {
    NS_LOG_INFO("Error Address not supported");
  }

  //printf("Sent %d , count %d\n",m_sent, m_count);
  if (m_sent < m_count)
  {
    m_interval = SetInterval();
    ScheduleTransmit(m_interval);
  }
}

void RpcClient::SetIntervalRatio(double ratio)
{
  m_intervalRatio = ratio;
}

Time RpcClient::SetInterval()
{
  Time interval;
  switch (m_dist)
  {
  case nodist:
  {
    interval = m_interval;
    break;
  }
  case incremental:
  {
    double nextTime = incrementalDistributionNext((double)m_interval.GetSeconds(), m_intervalRatio);
    //printf("Current Interval - %f, next Interval %f\n",(float)m_interval.GetSeconds(), nextTime);
    interval = Time(Seconds(nextTime));
    break;
  }
  case evenuniform:
  {
    double nextTime = evenUniformDistributionNext(0, 0);
    interval = Time(Seconds(nextTime));
    break;
  }
  case exponential:
  {
    double nextTime = (double)exponentailDistributionNext(0, 0);
    interval = Time(Seconds(nextTime));
    break;
  }
  case possion:
  {
    double nextTime = (double)poissonDistributionNext(0, 0);
    interval = Time(Seconds(nextTime));
    break;
  }
  }
  return interval;
}

void RpcClient::HandleRead(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  //PdcpTag idtag;
  Ipv4PacketInfoTag idtag;
  Address from;
  while ((packet = socket->RecvFrom(from)))
  {
    if (packet->PeekPacketTag(idtag))
    {

      m_rec++;
      //NS_LOG_INFO("Tag ID" << idtag.GetSenderTimestamp());
      //NS_LOG_INFO("timestamp index " << idtag.GetSenderTimestamp().GetNanoSeconds());
      //int requestIndex = int(idtag.GetSenderTimestamp().GetNanoSeconds()) % REQUEST_BUFFER_SIZE;
      int requestIndex = int(idtag.GetRecvIf()) % REQUEST_BUFFER_SIZE;
      //TODO each packet should just be timestamped rather than keeping track of each request.
      Time difference = Simulator::Now() - m_requests[requestIndex];
      //NS_LOG_WARN(difference.GetNanoSeconds());
      //
      //       NS_LOG_WARN(difference.GetNanoSeconds() << "," <<
      //		       Simulator::Now ().GetSeconds ());

      NS_LOG_WARN(difference.GetNanoSeconds() << "," << Simulator::Now().GetSeconds() << "," << m_sent << "," << m_rec << "," << requestIndex << "," << 0 << "," << this->GetNode()->GetId() << ",");
    }
    if (InetSocketAddress::IsMatchingType(from))
    {
      NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s client received " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
    }
    else if (Inet6SocketAddress::IsMatchingType(from))
    {
      NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s client received " << packet->GetSize() << " bytes from " << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port " << Inet6SocketAddress::ConvertFrom(from).GetPort());
    }
  }
}

void RpcClient::SetDistribution(enum distribution dist)
{
  m_dist = dist;
}

double RpcClient::incrementalDistributionNext(double current, double rate)
{
  double nextRate = current * rate;
  double percentbound = nextRate * 0.1;
  double fMin = 0.0 - percentbound;
  double fMax = 0.0 + percentbound;
  double f = (double)rand() / RAND_MAX;
  double offset = fMin + f * (fMax - fMin);
  double ret = nextRate + offset;
  //printf("New Rate %f\n",ret);
  return ret;
}

int RpcClient::evenUniformDistributionNext(int min, int max)
{
  return 0;
}

int RpcClient::exponentailDistributionNext(int min, int max)
{
  return 0;
}

int RpcClient::poissonDistributionNext(int min, int max)
{
  return 0;
}

} // Namespace ns3
