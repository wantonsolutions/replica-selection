#include "ipv4-doppelganger-tag.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4DoppelgangerTag");

Ipv4DoppelgangerTag::Ipv4DoppelgangerTag() {}

TypeId
Ipv4DoppelgangerTag::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Ipv4DoppelgangerTag")
                          .SetParent<Tag>()
                          .SetGroupName("Internet")
                          .AddConstructor<Ipv4DoppelgangerTag>();
  return tid;
}

void
Ipv4DoppelgangerTag::SetCanRouteDown(bool CanRouteDown)
{
  m_can_route_down = (uint8_t) CanRouteDown;
}

bool 
Ipv4DoppelgangerTag::GetCanRouteDown() {
  return (bool) m_can_route_down;
}


void 
Ipv4DoppelgangerTag::SetPacketType(Ipv4DoppelgangerTag::PacketType type){
  m_packet_type=(uint8_t) type;
}


Ipv4DoppelgangerTag::PacketType 
Ipv4DoppelgangerTag::GetPacketType(){
  return (Ipv4DoppelgangerTag::PacketType) m_packet_type;
}

void 
Ipv4DoppelgangerTag::SetRequestID(uint16_t requestID)
{
  m_requestID = requestID;
}
uint16_t
Ipv4DoppelgangerTag::GetRequestID(void) const
{
  return m_requestID;
}

void Ipv4DoppelgangerTag::SetPacketID(uint16_t packetID)
{
  m_packetID = packetID;
}

uint16_t
Ipv4DoppelgangerTag::GetPacketID(void) const
{
  return m_packetID;
}

void Ipv4DoppelgangerTag::SetReplicas(uint32_t replicas[MAX_REPLICAS])
{
  for (int i = 0; i < MAX_REPLICAS; i++)
  {
    m_replicas[i] = replicas[i];
  }
}

  void 
  Ipv4DoppelgangerTag::SetReplica(uint32_t index, uint32_t replicaAddress) {
    if (index >= MAX_REPLICAS) {
      NS_LOG_WARN("Unable to set replica, index " << index << " is out of range of MAX_REPLICAS " << MAX_REPLICAS);
      return;
    }
    m_replicas[index] = replicaAddress;
  }

  uint32_t
  Ipv4DoppelgangerTag::GetReplica(uint32_t index){
    if (index >= MAX_REPLICAS) {
      NS_LOG_WARN("Unable to get replica, index " << index << " is out of range of MAX_REPLICAS " << MAX_REPLICAS);
      return 0;
    }
    return m_replicas[index];
  }

uint32_t *
Ipv4DoppelgangerTag::GetReplicas(void) const
{
  return (uint32_t *)m_replicas;
}

void 
Ipv4DoppelgangerTag::SetHostSojournTime(uint64_t time){
  m_host_sojourn_time=time;
}

uint64_t 
Ipv4DoppelgangerTag::GetHostSojournTime(){
  return m_host_sojourn_time;
}

TypeId
Ipv4DoppelgangerTag::GetInstanceTypeId(void) const
{
  return GetTypeId();
}

uint32_t
Ipv4DoppelgangerTag::GetSerializedSize(void) const
{
  return sizeof(uint8_t) +
        sizeof(uint8_t) +
        sizeof(uint16_t) +
        sizeof(uint16_t) +
        (sizeof(uint32_t) * MAX_REPLICAS) +
        sizeof(uint64_t);
}

void Ipv4DoppelgangerTag::Serialize(TagBuffer i) const
{
  i.WriteU8(m_can_route_down);
  i.WriteU8(m_packet_type);
  i.WriteU16(m_requestID);
  i.WriteU16(m_packetID);
  for (int itt = 0; itt < MAX_REPLICAS; itt++)
  {
    i.WriteU32(m_replicas[itt]);
  }
  i.WriteU64(m_host_sojourn_time);
}

void Ipv4DoppelgangerTag::Deserialize(TagBuffer i)
{
  m_can_route_down = i.ReadU8();
  m_packet_type = i.ReadU8();
  m_requestID = i.ReadU16();
  m_packetID = i.ReadU16();
  for (int itt = 0; itt < MAX_REPLICAS; itt++)
  {
    m_replicas[itt] = i.ReadU32();
  }
  m_host_sojourn_time = i.ReadU64();
}

void Ipv4DoppelgangerTag::Print(std::ostream &os) const
{
  os << "Can Route Down = " << m_can_route_down;
  os << "Packet Type = " << m_packet_type;
  os << "request ID = " << m_requestID;
  os << "packet ID  = " << m_packetID;
  for (int itt = 0; itt < MAX_REPLICAS; itt++)
  {
    os << "Replica " << itt << " " << m_replicas[itt];
  }
  os << "host sojourn time = "<< m_host_sojourn_time;
}

} // namespace ns3
