
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
#include "ns3/ipv4-doppelganger-tag.h"
#include "ns3/ipv4-doppelganger-routing.h"


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
                                        UintegerValue(20000),
                                        MakeUintegerAccessor(&RpcClient::m_count),
                                        MakeUintegerChecker<uint32_t>())
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

void RpcClient::SetLocalPort(uint16_t local_port) {
  m_local_port = local_port;
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
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_local_port);
      if (m_socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
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

void RpcClient::SetGlobalSeverLoad(uint64_t *serverLoad) {
  m_serverLoad = serverLoad;
}

void RpcClient::SetGlobalServerLoadLog(std::vector<std::vector<LoadEvent>> *global_load_log) {
  m_global_load_log = global_load_log;
}

void RpcClient::SetGlobalSeverLoadUpdate(Time *serverLoad_update) {
  m_serverLoad_update = serverLoad_update;
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
    NS_LOG_WARN("Error too many replicas for a single RPC header packet [service: " << rpc_service << " services: " << m_rpc_server_replicas[rpc_service].size() << " max: " << MAX_REPLICAS << "]");
  }
  for( uint i =0; i< m_rpc_server_replicas[rpc_service].size(); i++) {
    Ipv4Address addr = Ipv4Address::ConvertFrom(m_peerAddresses[m_rpc_server_replicas[rpc_service][i]]);
    rpch->Replicas[i] = addr.Get();
  }
}

void RpcClient::PopulateReplicasReplicas(Ipv4DoppelgangerTag *idgt) {
  int rpc_service = idgt->GetRequestID();
  if (m_rpc_server_replicas[rpc_service].size() > MAX_REPLICAS) {
    NS_LOG_WARN("Error too many replicas for a single RPC header packet [service: " << rpc_service << " services: " << m_rpc_server_replicas[rpc_service].size() << " max: " << MAX_REPLICAS << "]");
  }
  for( uint i =0; i< m_rpc_server_replicas[rpc_service].size(); i++) {
    Ipv4Address addr = Ipv4Address::ConvertFrom(m_peerAddresses[m_rpc_server_replicas[rpc_service][i]]);
    idgt->SetReplica(i,addr.Get());
  }
}

int RpcClient::FindReplicaAddress(int rpc) {
  int replicaServerAddress;
  switch (m_selection_strategy) {
    case noReplica:
      replicaServerAddress = replicaSelectionStrategy_firstIndex(rpc);
      break;
    case randomReplica:
      replicaServerAddress = replicaSelectionStrategy_random(rpc);
      break;
    case minimumReplica:
      replicaServerAddress = replicaSelectionStrategy_minimumLoad(rpc);
      break;
    default:
      replicaServerAddress = -1;
      NS_LOG_WARN("strategy " << m_selection_strategy << " is invalid ");
      break;
  }
  return replicaServerAddress;

}

void RpcClient::SetReplicaSelectionStrategy(selectionStrategy strategy){
  m_selection_strategy = strategy;
}


  void RpcClient::SetInformationDelayFunction(InformationDelayFunction delay_function){
    m_delay_function = delay_function;
  }
  InformationDelayFunction RpcClient::GetInformationDelayFunction() {
    return m_delay_function;
  }

  void RpcClient::SetConstantDelay(uint64_t delay) {
    m_constant_information_delay = delay;
  }

  uint64_t RpcClient::GetConstantDelay() {
    return m_constant_information_delay;
  }

 int RpcClient::replicaSelectionStrategy_firstIndex(int rpc){
   return m_rpc_server_replicas[rpc][0];
 }
 int RpcClient::replicaSelectionStrategy_random(int rpc) {
   int randomReplica = rand()%m_rpc_server_replicas[rpc].size();
   return m_rpc_server_replicas[rpc][randomReplica];
 }

 uint64_t RpcClient::GetInformationTime() 
{
  return GetInformationDelay(m_delay_function,m_constant_information_delay);
}

 int RpcClient::replicaSelectionStrategy_minimumLoad(int rpc){
   uint64_t minLoad = UINT64_MAX;
   uint64_t minReplica;
   //printf("accessing min map for rpc %d\n", rpc);
   //printf("why is this not being built?\n");
   uint64_t time = GetInformationTime(); //This will likely have to be extended 
   for (uint i = 0; i < m_rpc_server_replicas[rpc].size();i++) {
     int replica = m_rpc_server_replicas[rpc][i];
     //printf("checking serverload for server ID %d\n", replica);
     uint64_t dialated_load = ServerLoadAtTime(replica,time,m_global_load_log);
     /*
     uint64_t sanity_load = GetInstantenousLoad(replica);
     if (dialated_load != sanity_load) {
       NS_LOG_WARN("What on earth, how can the dialted of current time not be the same as instant load (dialated = " << dialated_load << ") ( sanity " << sanity_load << ")");

     }*/
     if (dialated_load < minLoad){
       minLoad = dialated_load;
       minReplica = replica;
     }
     //printf("done loop server ID %d\n", replica);
   }
   //printf("done accessing min map\n");
   return minReplica;
 }

uint64_t
RpcClient::GetInstantenousLoad(int server_id) {
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

 void RpcClient::SetPacketSizeDistribution(std::vector<uint32_t> packetSizes) {
   m_packet_size_distribution = packetSizes;
 }
 std::vector<uint32_t> RpcClient::GetPacketSizeDistribution() {
   return m_packet_size_distribution;
 }

 void RpcClient::SetTransmitionDistribution(std::vector<uint32_t> transmissionDistribution) {
   m_transmission_distribution = transmissionDistribution;
 }

 std::vector<uint32_t> RpcClient::GetPacketTransmissionDistribution() {
   return m_transmission_distribution;
 }

 void RpcClient::SetRPCDistribution(std::vector<uint32_t> rpcDistribution) {
   m_rpc_request_distribution = rpcDistribution;
 }

 std::vector<uint32_t> RpcClient::GetRPCDistribution() {
   return m_rpc_request_distribution;
 }

 uint32_t RpcClient::GetNextPacketSize() {
   if (m_packet_size_distribution.empty()) {
     NS_LOG_WARN(this << "Packet Size Distribution Not Set - returning packet size of 0");
     return 0;
   }
   //TODO change rand to rand seed for future experiments
   int index = rand() % m_packet_size_distribution.size();
   return m_packet_size_distribution[index];
 }

 Time RpcClient::GetNextTransmissionInterval() {
   if (m_transmission_distribution.empty()) {
     NS_LOG_WARN(this << "Transmision distribution not set - returning transmission of 0 (should break output)");
   }
   //TODO change rand to rand seed for future experiments
   int index = rand() % m_transmission_distribution.size();
   //NS_LOG_WARN("Scheduing next interval in " << m_transmission_distribution[index]);
   return Time(m_transmission_distribution[index]);
 }
 
 uint32_t RpcClient::GetNextRPC() {
   if (m_rpc_request_distribution.empty()) {
     NS_LOG_WARN(this << "RPC request distribution not set - returning 0 (this will likely cause huge incast)");
     return 0;
   }
   //TODO change rand to rand seed for future experiments
   uint32_t index = m_rpc_request_distribution[rand() % m_rpc_request_distribution.size()];

   if(index >= m_rpc_server_replicas.size()) {
     NS_LOG_WARN(this << " server RPC " << index << " requested is out of range of the known services, check the generation code for RPC generation, (returning 0, this will likely cause incast");
     return 0;
   }
   return index;
 }


void RpcClient::Send(void)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_sendEvent.IsExpired());


  //Generate a packet based on the size of the next request
  //TODO generate a schema by which multiple packets are sent per request
  Ptr<Packet> p;
  uint32_t packet_size = GetNextPacketSize();
  p = Create<Packet>(packet_size);


  Ipv4DoppelgangerTag ipv4DoppelgangerTag;
  
  // PacketID is an analog for sequence number, for now request are a single packet so always set to 0
  ipv4DoppelgangerTag.SetCanRouteDown(false);
  ipv4DoppelgangerTag.SetPacketType(Ipv4DoppelgangerTag::request);
  ipv4DoppelgangerTag.SetPacketID(0); 
  ipv4DoppelgangerTag.SetHostSojournTime(0);
  ipv4DoppelgangerTag.SetRedirections(0);

  // Generate the location of the next reuqest
  uint32_t request_id = GetNextRPC();
  ipv4DoppelgangerTag.SetRequestID(request_id);
  PopulateReplicasReplicas(&ipv4DoppelgangerTag);
  p->AddPacketTag(ipv4DoppelgangerTag);

  //Set the global request number of the packet for latency tracking
  Ipv4PacketInfoTag idtag;
  uint32_t send_index = m_sent % REQUEST_BUFFER_SIZE;
  idtag.SetRecvIf(send_index);
  p->AddPacketTag(idtag);
  m_requests[m_sent % REQUEST_BUFFER_SIZE] = Simulator::Now();

  //Find Replicas of the RPC
  m_txTrace(p);

  //Use a strategy to find the address of the next replica server
  int replicaServerAddress = FindReplicaAddress(ipv4DoppelgangerTag.GetRequestID());
  //int replica = replicaSelectionStrategy_minimumLoad(rpch.RequestID);
  m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddresses[replicaServerAddress]), m_peerPort));
  m_socket->Send(p);

  ++m_sent;
  ++(*m_global_sent);
  if (Ipv4Address::IsMatchingType(m_peerAddresses[replicaServerAddress]))
  {
    NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s client sent local packet "<< m_sent << " gloabal packet " << *m_global_sent << " of size " << m_size << " bytes to " << Ipv4Address::ConvertFrom(m_peerAddresses[ipv4DoppelgangerTag.GetRequestID()]) << " port " << m_peerPort);
  } else {
    NS_LOG_INFO("Error Address not supported");
  }

  //printf("Sent %d , count %d\n",m_sent, m_count);
  if (m_sent < m_count)
  {
    ScheduleTransmit(GetNextTransmissionInterval());
  }
}

void RpcClient::HandleRead(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  //PdcpTag idtag;
  Ipv4PacketInfoTag idtag;
  Ipv4DoppelgangerTag ipv4DoppelgangerTag;


  Address from;
  while ((packet = socket->RecvFrom(from)))
  {
    if (packet->PeekPacketTag(idtag))
    {
      if (packet->PeekPacketTag(ipv4DoppelgangerTag)){
        NS_LOG_INFO("Received Packet with correct doppel tag");
      } else {
        NS_LOG_INFO("Received packet, but there is no doppelganger tag, there is likely a bug, doing nothing");
        return;
      }

      if (ipv4DoppelgangerTag.GetPacketType() == Ipv4DoppelgangerTag::load) {
          //server_local_load[id] == ipv4DoppelgangerTag.GetHostLoad();
          NS_LOG_INFO("Got Host load of " << ipv4DoppelgangerTag.GetHostLoad() << " from " << InetSocketAddress::ConvertFrom(from).GetIpv4() );
      }

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

      NS_LOG_WARN(difference.GetNanoSeconds() << "," << Simulator::Now().GetSeconds() << "," << m_sent << "," << m_rec << "," << requestIndex << "," << 0 << "," << this->GetNode()->GetId() << "," << ipv4DoppelgangerTag.GetHostSojournTime());
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

}