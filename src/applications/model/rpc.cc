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

#include "ns3/ipv4-packet-info-tag.h"
#include "rpc.h"

namespace ns3 {

void DecodeRPCHeader(RPCHeader* decodeTo, char * encodedBytes){
  memcpy((char*) decodeTo, encodedBytes, sizeof(RPCHeader));
}

void EncodeRPCHeader(char* toEncode, RPCHeader* header){
  memcpy(toEncode, (char*) header, sizeof(RPCHeader));
}

}