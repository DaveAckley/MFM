#ifndef TILE_H      /* -*- C++ -*- */
#define TILE_H

#include "eucliddir.h"
#include "random.h"  /* for Random */
#include "packet.h"
#include "packetbuffer.h"
#include "point.h"
#include "eventwindow.h"
#include "elementtable.h"


namespace MFM {

/* The full length, in sites, of a Tile, including neighbor caches.*/
#define TILE_WIDTH 40

/*The number of sites a tile contains.*/
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)

  template <class T,u32 R>
class Tile
{
public:
  static const u32 EVENT_WINDOW_RADIUS = R;

  static const u32 ELEMENT_TABLE_BITS = 8;  // Currently hard-coded!
  static const u32 ELEMENT_TABLE_SIZE = 1u<<ELEMENT_TABLE_BITS;

  /**
   * The edge length of the portion of a tile that is 'owned' by the
   * tile itself -- i.e., excluding the cache boundary.
   */
  static const u32 OWNED_SIDE = TILE_WIDTH-2*R;

private:

  ElementTable<T,R,ELEMENT_TABLE_BITS> elementTable;

  Random m_random;

  u64 m_eventsExecuted;

  u8 m_neighborConnections;

  friend class EventWindow<T,R>;

  T m_atoms[TILE_SIZE];

  s32 m_atomCount[ELEMENT_TABLE_SIZE];

  SPoint m_lastExecutedAtom;

  EventWindow<T,R> m_executingWindow;

  PacketBuffer<T> m_outgoingPackets;

  void CreateRandomWindow();

  void CreateWindowAt(const SPoint& pt);

  //  u32 (*m_stateFunc)(T* atom);

  void SendRelevantAtoms();

  void SendAtom(EuclidDir neighbor, SPoint& atomLoc);

public:

  Tile();

  u32 GetTileWidth() 
  {
    return TILE_WIDTH;
  }

  Random & GetRandom();

  void SetNeighbors(u8 neighbors);

  ElementTable<T,R,ELEMENT_TABLE_BITS> & GetElementTable() {
    return elementTable;
  }

  inline bool IsConnected(EuclidDir dir);

  static inline bool IsInCache(SPoint& pt);

  u32 GetEventsExecuted() const 
  {
    return m_eventsExecuted;
  }

  /*
  void SetStateFunc(u32 (*stateFunc)(T* atom))
  {
    m_stateFunc = stateFunc;
  }

  u32 (*GetStateFunc())(T* atom)
  {
    return m_stateFunc;
  }
  */

  /*
   * Finds the cache pointed at by pt. If there
   * is no cache there, this will return -1.
   */
  EuclidDir CacheAt(SPoint& pt);


  /**
   * Expose the underlying atom array because, e.g., testing depends
   * on it?  Urgh?
   */
  T* GetAtoms() ;

  T* GetAtom(const SPoint& pt);

  T* GetAtom(int x, int y);

  T* GetAtom(int i);

  void ReceivePacket(Packet<T>& packet);

  /*
   * Gets the next packet that needs to be sent. This
   * returns NULL if there are no more packets to take.
   */
  Packet<T>* NextPacket();

  void FillLastExecutedAtom(SPoint& out);

  /**
   * Store an atom anywhere in the tile, using the 'raw' coordinate
   * system in which (0,0) is in the tile cache.  See also PlaceInternalAtom
   */
  void PlaceAtom(const T& atom, const SPoint& pt);

  /**
   * Store an atom in the 'locally-owned' portion of a tile, using the
   * 'natural' coordinate system in which (0,0) is the upper left of
   * the sites that this tile owns..  See also PlaceAtom
   */
  void PlaceInternalAtom(const T& atom, const SPoint& pt) {
    PlaceAtom(atom,pt+SPoint(R,R));
  }

  void Execute();

  u32 GetAtomCount(ElementType atomType);

  void SetAtomCount(ElementType atomType, u32 count);

  void IncrAtomCount(ElementType atomType, s32 delta);

  void RegisterElement(const Element<T,R> & anElement) {
    elementTable.RegisterElement(anElement);
  }
};
} /* namespace MFM */

#include "tile.tcc"

#endif /*TILE_H*/

