#ifndef TILE_H      /* -*- C++ -*- */
#define TILE_H

#include "eucliddir.h"
#include "p1atom.h"
#include "packet.h"
#include "packetbuffer.h"
#include "point.h"
#include "eventwindow.h"
#include "elementtable.h"

/* The length, in sites, of a Tile.*/
#define TILE_WIDTH 40

/*The number of sites a tile contains.*/
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)

template <class T,u32 R>
class Tile
{
  static const u32 EVENT_WINDOW_RADIUS = R;

private:

  u8 m_neighborConnections;

  T m_atoms[TILE_SIZE];

  s32 m_atomCount[ELEMENT_COUNT];

  Point<int> m_lastExecutedAtom;

  EventWindow<T,R> m_executingWindow;

  PacketBuffer<T> m_outgoingPackets;

  void CreateRandomWindow();

  void CreateWindowAt(Point<int>& pt);

  u32 (*m_stateFunc)(T* atom);

  /*
   * Finds the cache pointed at by pt. If there
   * is no cache there, this will return -1.
   */
  EuclidDir CacheAt(Point<int>& pt);

  void SendRelevantAtoms();

  void SendAtom(EuclidDir neighbor, Point<int>& atomLoc);

  inline bool IsConnected(EuclidDir dir);

  void AddToAtomCounts(s32 value);

  void RemovePreWindowAtomCount();

  void AddPostWindowAtomCount();

public:

  Tile();

  void SetNeighbors(u8 neighbors);

  static inline bool IsInCache(Point<int>& pt);

  void SetStateFunc(u32 (*stateFunc)(T* atom))
  {
    m_stateFunc = stateFunc;
  }

  u32 (*GetStateFunc())(T* atom)
  {
    return m_stateFunc;
  }

  T* GetAtom(Point<int>* pt);

  T* GetAtom(int x, int y);

  T* GetAtom(int i);

  void ReceivePacket(Packet<T>& packet);

  /*
   * Gets the next packet that needs to be sent. This
   * returns NULL if there are no more packets to take.
   */
  Packet<T>* NextPacket();

  void FillLastExecutedAtom(Point<int>& out);

  void PlaceAtom(T& atom, Point<int>& pt);

  void DiffuseAtom(EventWindow<T,R>& window);

  void Execute(ElementTable<T,R>& table);

  u32 GetAtomCount(ElementType atomType);
};

#include "tile.tcc"

#endif /*TILE_H*/
