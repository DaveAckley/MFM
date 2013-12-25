#ifndef TILE_H
#define TILE_H

#include "eucliddir.h"
#include "p1atom.h"
#include "point.h"
#include "eventwindow.h"
#include "elementtable.hpp"

/* The length, in sites, of a Tile.*/
#define TILE_WIDTH 20

/*The number of sites a tile contains.*/
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)

template <class T>
class Tile
{
private:
  T m_atoms[TILE_SIZE];

  EventWindow<T>* CreateRandomWindow();

  u32 (*m_stateFunc)(T* atom);

public:

  Tile() { }

  void SetStateFunc(u32 (*stateFunc)(T* atom))
  { m_stateFunc = stateFunc; }

  u32 (*GetStateFunc())(T* atom)
  {
    return m_stateFunc;
  }

  T* GetAtom(Point<int>* pt);

  T* GetAtom(int x, int y);

  T* GetAtom(int i);

  void PlaceAtom(T* atom, Point<int>* pt);

  void Execute();

  void ResetAtom(Point<int>* point,
		 ElementType type);

};

#include "tile.tcc"

#endif /*TILE_H*/
