#ifndef NS3_IPV4_DOPPELGANGER_TAG
#define NS3_IPV4_DOPPELGANGER_TAG

#define MAX_REPLICAS 5
#define KTAG 4
#include "ns3/tag.h"

namespace ns3 {

class Ipv4DoppelgangerTag: public Tag
{
public:


    enum PacketType
    {
      request = 0,
      response = 1,
      load = 2,
      response_piggyback = 3,
      tor_to_tor_load = 4,
    };


    Ipv4DoppelgangerTag ();

    static TypeId GetTypeId (void);

    void SetCanRouteDown(bool CanRoute);
    bool GetCanRouteDown();

    void SetPacketType(PacketType type);
    PacketType GetPacketType();

    void SetReplicaCount(uint8_t replica_count);
    uint8_t GetReplicaCount();

    void SetRequestID(uint16_t requestID);
    uint16_t GetRequestID(void) const;

    void SetPacketID(uint16_t packetID);
    uint16_t GetPacketID(void) const;

    void SetReplicas(uint32_t replicas[MAX_REPLICAS]);
    uint32_t* GetReplicas(void)const;

    void SetReplica(uint32_t index, uint32_t replicaAddress);
    uint32_t GetReplica(uint32_t index);

    void SetTorQueueDepth(uint32_t index, uint32_t serverAddress, uint32_t depth);
    uint32_t GetTorReplica(uint32_t index);
    uint8_t GetTorReplicaQueueDepth(uint32_t index);
    void SetTorQueuesNULL();
    bool TorQueuesAreNULL();

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

    uint8_t m_replica_count;
    uint8_t m_can_route_down;
    uint8_t m_packet_type;
    uint16_t m_requestID;
    uint16_t m_packetID;
    uint32_t m_replicas[MAX_REPLICAS];
    uint64_t m_host_sojourn_time;
    uint64_t m_host_load;
    uint8_t m_redirections;
    uint32_t m_tor_ip[KTAG/2];
    uint8_t m_tor_ip_queue_depth[KTAG/2];
};



struct RPCHeader {
  //TODO refactor RequestID to RPCType
  //TODO add RequestID hash
  //TODO size_id
  uint8_t ReplicaCount;
  uint8_t CanRouteDown;
  uint8_t PacketType;
  uint16_t RequestID; //RPC ID 
  uint16_t PacketID;  //Sequence Number
  uint32_t Replicas[MAX_REPLICAS]; //IP addresses of replicas
  uint64_t HostSojournTime;
  uint64_t HostLoad;
  uint8_t Redirections;
  uint32_t TorIP[KTAG/2];
  uint8_t TorIPQueueDepth[KTAG/2];

};

void DecodeRPCHeader(RPCHeader* decodeTo, char * encodedBytes);
void EncodeRPCHeader(char* toEncode, RPCHeader* header);

}

#endif
