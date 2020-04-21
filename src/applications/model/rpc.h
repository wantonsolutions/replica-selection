#ifndef RPC_H
#define RPC_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/socket.h"


#define MAX_REPLICAS 2

namespace ns3 {



struct RPCHeader {
  uint16_t RequestID; //RPC ID 
  uint16_t PacketID; //Sequence Number
  uint32_t Replicas[MAX_REPLICAS]; //IP addresses of replicas
};

void DecodeRPCHeader(RPCHeader* decodeTo, char * encodedBytes);
void EncodeRPCHeader(char* toEncode, RPCHeader* header);

/*
struct RaidState {
  bool **Served_Raid_Requests;
  Ptr<Packet> **Served_Raid_Packets;
};

RaidState* InitRaidState(int parallel);
Ptr<Packet> RaidReceive(Ptr<Packet> packet, Address from, RaidState *rs, int parallel);
void RaidWrite(int requestIndex,RaidState *rs, Ptr<Packet>* packets, Ptr<Socket> socket, Address to, int parallel);
int GetHitIndex(Address from, int requestIndex);
Ptr<Packet>* StripePacket(int parallel, uint32_t size, uint32_t sent, uint8_t* data);
int GetRaidFlowState(int requestIndex, int parallel, RaidState *rs);
Ptr<Packet> FixPacket(int requestIndex, int parallel, RaidState *rs);
Ptr<Packet> MergePacket(int requestIndex, int parallel, RaidState *rs);
*/

} // namespace ns3

#endif /* RAID_CLIENT_H */
