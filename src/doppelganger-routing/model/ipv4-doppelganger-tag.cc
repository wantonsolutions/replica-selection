#include "ipv4-doppelganger-tag.h"

namespace ns3
{

Ipv4DoppelgangerTag::Ipv4DoppelgangerTag () {}

TypeId
Ipv4DoppelgangerTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv4DoppelgangerTag")
    .SetParent<Tag> ()
    .SetGroupName ("Internet")
    .AddConstructor<Ipv4DoppelgangerTag> ();
  return tid;
}

void
Ipv4DoppelgangerTag::SetLbTag (uint32_t lbTag)
{
  m_lbTag = lbTag;
}

uint32_t
Ipv4DoppelgangerTag::GetLbTag (void) const
{
  return m_lbTag;
}

void
Ipv4DoppelgangerTag::SetCe (uint32_t ce)
{
  m_ce = ce;
}

uint32_t
Ipv4DoppelgangerTag::GetCe (void) const
{
  return m_ce;
}

void
Ipv4DoppelgangerTag::SetFbLbTag (uint32_t fbLbTag)
{
  m_fbLbTag = fbLbTag;
}

uint32_t
Ipv4DoppelgangerTag::GetFbLbTag (void) const
{
  return m_fbLbTag;
}

void
Ipv4DoppelgangerTag::SetFbMetric (uint32_t fbMetric)
{
  m_fbMetric = fbMetric;
}

uint32_t
Ipv4DoppelgangerTag::GetFbMetric (void) const
{
  return m_fbMetric;
}

TypeId
Ipv4DoppelgangerTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
Ipv4DoppelgangerTag::GetSerializedSize (void) const
{
  return sizeof (uint32_t) +
         sizeof (uint32_t) +
         sizeof (uint32_t) +
         sizeof (uint32_t);
}

void
Ipv4DoppelgangerTag::Serialize (TagBuffer i) const
{
  i.WriteU32(m_lbTag);
  i.WriteU32(m_ce);
  i.WriteU32(m_fbLbTag);
  i.WriteU32(m_fbMetric);
}

void
Ipv4DoppelgangerTag::Deserialize (TagBuffer i)
{
  m_lbTag = i.ReadU32 ();
  m_ce = i.ReadU32 ();
  m_fbLbTag = i.ReadU32 ();
  m_fbMetric = i.ReadU32 ();

}

void
Ipv4DoppelgangerTag::Print (std::ostream &os) const
{
  os << "Lb Tag = " << m_lbTag;
  os << "CE  = " << m_ce;
  os << "Feedback Lb Tag = " << m_fbLbTag;
  os << "Feedback Metric = " << m_fbMetric;
}

}
