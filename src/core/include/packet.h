#ifndef PACKET_H /* -*- C++ -*- */
#define PACKET_H

#include "eucliddir.h"
#include "itype.h"

namespace MFM {

/* 
 * The basic kinds of packets that
 * will be used between tiles for
 * comunication.
 */
typedef enum
{
  PACKET_WRITE,
  PACKET_LOCK_REQUEST,
  PACKET_LOCK_ACCEPT,
  PACKET_UNLOCK
}PacketType;

template <class T>
class Packet
{
private:
  PacketType m_type;
  
  SPoint m_edgeLoc;

  EuclidDir m_toNeighbor;

  T m_atom;

public:

  Packet(PacketType type);

  Packet() { }

  PacketType GetType()
  { return m_type; }

  void SetType(PacketType type)
  { m_type = type; }

  void SetAtom(T& atom);

  void SetReceivingNeighbor(EuclidDir dir)
  { m_toNeighbor = dir; }

  EuclidDir GetReceivingNeighbor()
  { return m_toNeighbor; }

  T& GetAtom();

  void SetLocation(const SPoint& fromPt);

  const SPoint& GetLocation() const;
};
} /* namespace MFM */

#include "packet.tcc"

#endif /*PACKET_H*/

