#ifndef NS3_IPV4_DOPPELGANGER_TAG
#define NS3_IPV4_DOPPELGANGER_TAG

#define MAX_REPLICAS 2

#include "ns3/tag.h"

namespace ns3 {

class Ipv4DoppelgangerTag: public Tag
{
public:

    enum PacketType
    {
      request = 0,
      response = 1,
    };

    Ipv4DoppelgangerTag ();

    static TypeId GetTypeId (void);

    void SetCanRouteDown(bool CanRoute);
    bool GetCanRouteDown();

    void SetPacketType(PacketType type);
    PacketType GetPacketType();

    void SetRequestID(uint16_t requestID);
    uint16_t GetRequestID(void) const;

    void SetPacketID(uint16_t packetID);
    uint16_t GetPacketID(void) const;

    void SetReplicas(uint32_t replicas[MAX_REPLICAS]);
    uint32_t* GetReplicas(void)const;

    void SetReplica(uint32_t index, uint32_t replicaAddress);
    uint32_t GetReplica(uint32_t index);

    void SetHostSojournTime(uint64_t time);
    uint64_t GetHostSojournTime();

    void SetHostLoad(uint64_t load);
    uint64_t GetHostLoad();

    void SetRedirections(uint8_t redirections);
    uint8_t GetRedirections();

    virtual TypeId GetInstanceTypeId (void) const;

    virtual uint32_t GetSerializedSize (void) const;

    virtual void Serialize (TagBuffer i) const;

    virtual void Deserialize (TagBuffer i);

    virtual void Print (std::ostream &os) const;

private:

    uint8_t m_can_route_down;
    uint8_t m_packet_type;
    uint16_t m_requestID;
    uint16_t m_packetID;
    uint32_t m_replicas[MAX_REPLICAS];
    uint64_t m_host_sojourn_time;
    uint64_t m_host_load;
    uint8_t m_redirections;
};



struct RPCHeader {
  //TODO refactor RequestID to RPCType
  //TODO add RequestID hash
  //TODO size_id
  uint8_t CanRouteDown;
  uint8_t PacketType;
  uint16_t RequestID; //RPC ID 
  uint16_t PacketID;  //Sequence Number
  uint32_t Replicas[MAX_REPLICAS]; //IP addresses of replicas
  uint64_t HostSojournTime;
  uint64_t HostLoad;
  uint8_t Redirections;
};

void DecodeRPCHeader(RPCHeader* decodeTo, char * encodedBytes);
void EncodeRPCHeader(char* toEncode, RPCHeader* header);

}

#endif
