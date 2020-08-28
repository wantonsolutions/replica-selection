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

#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup rpc UdpEcho
 */

struct LoadEvent {
  uint64_t time;
  uint64_t load;
};

enum InformationSpreadingFunction {
  none = 0,
  periodic = 1,
  piggyback = 2,
};

uint64_t ServerLoadAtTime(uint server_id, uint64_t time, std::vector<std::vector<LoadEvent>> *global_load_log);

/**
 * \ingroup rpc
 * \brief A Rpc server
 *
 * Every packet received is sent back.
 */
class RpcServer : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  RpcServer ();
  virtual ~RpcServer ();

  bool CanServiceRPC(int rpc_id);
  void AddRpc(int rpc_id);
  void AssignRPC(std::vector<int> services);

  void SetGlobalLoad(uint64_t * serverLoad);
  void SetGlobalLoadUpdate(Time * serverLoad_update);
  void SetGlobalLoadLog(std::vector<std::vector<LoadEvent>>* global_load_log);
  void SetID(int id);

  void SetClientAddresses(uint32_t num_clients, uint16_t port, Address * addresses);    //Array of Peer Addresses

  void SetLoadDistribution(std::vector<uint32_t> loadDistribution);
  std::vector<uint32_t> GetLoadDistribution();
  void SpreadLoadInformation();


  uint32_t GetRequestLoad();

  uint32_t GetInstantenousLoad();


protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);


  void SendResponse(Ptr<Socket> socket, Ptr<Packet> packet, Address from, int load);
  void ScheduleResponse(Time dt,Ptr<Socket> socket, Ptr<Packet> packet, Address from, int load);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local multicast address

  int m_id; //localID
  uint64_t *m_serverLoad; // global server load in nanoseconds
  Time *m_serverLoad_update; //The last time load was observed on this server, used to calculate instantenous load.

  InformationSpreadingFunction m_information_function = periodic;
  uint64_t m_information_spread_period = 3500;

  uint64_t m_min_time_between_delays = 500000;
  uint64_t m_delay_time = 500000;
  uint64_t m_last_delay = 0;


  std::vector<Address> m_client_addresses;    //Array of Peer Addresses
  std::vector<uint16_t> m_client_ports;          //Corresponding array of peer ports


  std::vector<int> m_serviceable_rpcs;

  std::vector<uint32_t> m_load_distribution;
  std::vector<std::vector<LoadEvent>> *m_load_log;
};

} // namespace ns3

#endif /* RPC_SERVER_H */

