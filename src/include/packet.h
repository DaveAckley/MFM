#ifndef PACKET_H /* -*- C++ -*- */
#define PACKET_H

#include "itype.h"

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
  
  Point<int> m_edgeLoc;

  T m_atom;

public:

  Packet(PacketType type);

  void SetAtom(T& atom);

  T& GetAtom();

  void SetLocation(Point<int>& fromPt);

  Point<int>& GetLocation();
};

#include "packet.tcc"

#endif /*PACKET_H*/
