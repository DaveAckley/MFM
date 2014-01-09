#ifndef TILE_H      /* -*- C++ -*- */
#define TILE_H

#include "eucliddir.h"
#include "p1atom.h"
#include "packet.h"
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

  T m_atoms[TILE_SIZE];

  typedef void (Tile<T,R>::*ComFunction )(Packet<T>& packet);

  ComFunction m_comFunctions[8];

  Point<int> m_lastExecutedAtom;

  EventWindow<T,R> m_executingWindow;

  void CreateRandomWindow();

  void CreateWindowAt(Point<int>& pt);

  u32 (*m_stateFunc)(T* atom);

  /*
   * Finds the cache point at by pt. If there
   * is no cache there, this will return -1.
   */
  EuclidDir CacheAt(Point<int>& pt);

  void SendRelevantAtoms();

  void SendAtom(EuclidDir neighbor, Point<int>& atomLoc);

public:

  Tile();

  void SetStateFunc(u32 (*stateFunc)(T* atom))
  { m_stateFunc = stateFunc; }

  u32 (*GetStateFunc())(T* atom)
  {
    return m_stateFunc;
  }

  void AddComFunction(ComFunction func, EuclidDir dir)
  {
    m_comFunctions[dir] = func;
  }

  T* GetAtom(Point<int>* pt);

  T* GetAtom(int x, int y);

  T* GetAtom(int i);

  void ReceivePacket(Packet<T>& packet);

  void FillLastExecutedAtom(Point<int>& out);

  void PlaceAtom(T& atom, Point<int>& pt);

  void DiffuseAtom(EventWindow<T,R>& window);

  void Execute(ElementTable<T,R>& table);
};

#include "tile.tcc"

#endif /*TILE_H*/
