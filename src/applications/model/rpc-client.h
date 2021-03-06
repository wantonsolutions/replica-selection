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

#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4-doppelganger-tag.h"
#include "ns3/ipv4-doppelganger-routing.h"
#include "ns3/rpc-server.h"

#define REQUEST_BUFFER_SIZE 100000
namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup rcp
 * \brief A rpc client
 *
 * Every packet sent should be returned by the server and received here.
 */
class RpcClient : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  RpcClient ();

  virtual ~RpcClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Address ip, uint16_t port);
  /**
   * \brief set the remote address
   * \param addr remote address
   */
  void SetRemote (Address addr);

  /**
   * Set the data size of the packet (the number of bytes that are sent as data
   * to the server).  The contents of the data are set to unspecified (don't
   * care) by this call.
   *
   * \warning If you have set the fill data for the echo client using one of the
   * SetFill calls, this will undo those effects.
   *
   * \param dataSize The size of the echo data you want to sent.
   */
  void SetDataSize (uint32_t dataSize);

  /**
   * Get the number of data bytes that will be sent to the server.
   *
   * \warning The number of bytes may be modified by calling any one of the 
   * SetFill methods.  If you have called SetFill, then the number of 
   * data bytes will correspond to the size of an initialized data buffer.
   * If you have not called a SetFill method, the number of data bytes will
   * correspond to the number of don't care bytes that will be sent.
   *
   * \returns The number of data bytes.
   */
  uint32_t GetDataSize (void) const;

  /**
   * Set the data fill of the packet (what is sent as data to the server) to 
   * the zero-terminated contents of the fill string string.
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the size of the fill string -- this means that the PacketSize
   * attribute may be changed as a result of this call.
   *
   * \param fill The string to use as the actual echo data bytes.
   */
  void SetFill (std::string fill);

  /**
   * Set the data fill of the packet (what is sent as data to the server) to 
   * the repeated contents of the fill byte.  i.e., the fill byte will be 
   * used to initialize the contents of the data packet.
   * 
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataSize parameter -- this means that the PacketSize
   * attribute may be changed as a result of this call.
   *
   * \param fill The byte to be repeated in constructing the packet data..
   * \param dataSize The desired size of the resulting echo packet data.
   */
  void SetFill (uint8_t fill, uint32_t dataSize);

  /**
   * Set the data fill of the packet (what is sent as data to the server) to
   * the contents of the fill buffer, repeated as many times as is required.
   *
   * Initializing the packet to the contents of a provided single buffer is 
   * accomplished by setting the fillSize set to your desired dataSize
   * (and providing an appropriate buffer).
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataSize parameter -- this means that the PacketSize
   * attribute of the Application may be changed as a result of this call.
   *
   * \param fill The fill pattern to use when constructing packets.
   * \param fillSize The number of bytes in the provided fill pattern.
   * \param dataSize The desired size of the final echo data.
   */
  void SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize);


/**
 * If set to true the echo client will send across parallel channels
 **/
  void SetParallel(bool parallel);

// Custom types
  enum selectionStrategy
  {
    noReplica = 0,
    randomReplica = 1,
    minimumReplica = 2,
  };



  void SetAllAddresses(Address *addresses, uint16_t *ports, int **tm, uint32_t numPeers);
  void SetAllAddressesParallel(Address **addresses, uint16_t **ports, int **trafficMatrix, uint8_t parallel, uint32_t numPeers);
  void SetGlobalPackets(uint32_t * global_packets);

  void SetRpcServices(std::vector<std::vector<int>> rpcServices);
  void SetGlobalSeverLoad(uint64_t *serverLoad);
  void SetGlobalSeverLoadUpdate(Time *serverLoad_update);
  void SetGlobalServerLoadLog(std::vector<std::vector<LoadEvent>> *global_load_log);
  void SetReplicaSelectionStrategy(selectionStrategy strategy);

  void SetLocalPort(uint16_t);

  void SetInformationDelayFunction(InformationDelayFunction delay_function);
  InformationDelayFunction GetInformationDelayFunction();
  void SetConstantDelay(uint64_t delay);
  uint64_t GetConstantDelay();
  uint64_t GetInformationTime();



 void PopulateReplicasNoReplicas(RPCHeader *rpch);
 void PopulateReplicasReplicas(RPCHeader *rpch);

 void PopulateReplicasReplicas(Ipv4DoppelgangerTag *idgt);

 int FindReplicaAddress(int rpc);
 int replicaSelectionStrategy_firstIndex(int rpc);
 int replicaSelectionStrategy_random(int rpc);
 int replicaSelectionStrategy_minimumLoad(int rpc);

 uint64_t GetInstantenousLoad(int server_id);

 void SetPacketSizeDistribution(std::vector<uint32_t>);
 std::vector<uint32_t> GetPacketSizeDistribution();

 void SetTransmitionDistribution(std::vector<uint32_t>);
 std::vector<uint32_t> GetPacketTransmissionDistribution();

 void SetRPCDistribution(std::vector<uint32_t>);
 std::vector<uint32_t> GetRPCDistribution();


 uint32_t GetNextPacketSize(void);
 Time GetNextTransmissionInterval(void);
 uint32_t GetNextRPC(void);





protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Schedule the next packet transmission
   * \param dt time interval between packets.
   */
  void ScheduleTransmit (Time dt);
  /**
   * \brief Send a packet
   */
  void Send (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  uint32_t m_count; //!< Maximum number of packets the application will send
  uint32_t m_size; //!< Size of the sent packet

  uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
  uint8_t *m_data; //!< packet payload data

  uint32_t m_sent; //!< Counter for sent packets
  uint32_t *m_global_sent;
  uint32_t m_rec; //!< Counter for recevied packets
  Ptr<Socket> m_socket; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

  bool m_parallel; //true if running on a parallel fat-tree

  uint16_t m_local_port;

  uint32_t m_numPeers;                     //Total Number of peers
  Address **m_peerAddresses_parallel;    //Array of Peer Addresses
  uint16_t **m_peerPorts_parallel;          //Corresponding array of peer ports

  Address *m_peerAddresses;    //Array of Peer Addresses
  uint16_t *m_peerPorts;          //Corresponding array of peer ports

  int **m_tm;          //Traffic Matrix 


  uint64_t *m_serverLoad; // global server load
  Time *m_serverLoad_update; // global server load update

  std::vector<std::vector<int>> m_rpc_server_replicas;
  int m_selection_strategy;

  std::vector<uint32_t> m_packet_size_distribution;
  std::vector<uint32_t> m_transmission_distribution;;
  std::vector<uint32_t> m_rpc_request_distribution;

  std::vector<std::vector<LoadEvent>>  *m_global_load_log;

  InformationDelayFunction m_delay_function;
  uint64_t m_constant_information_delay;

  Time m_requests[REQUEST_BUFFER_SIZE];

  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet> > m_txTrace;
};

} // namespace ns3

#endif /* RPC_CLIENT_H */
