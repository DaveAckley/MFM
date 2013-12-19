#ifndef TILE_H
#define TILE_H

#include "p1atom.h"
#include "point.h"
#include "eventwindow.h"

/* The length, in sites, of a Tile.*/
#define TILE_WIDTH 16

/*The number of sites a tile contains.*/
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)

class EventWindow;
template <class T>
class Tile
{
private:
  T m_atoms[TILE_SIZE];
  Point<int> m_location;

public:

  Tile(Point<int>* location);

  Tile(int x, int y);

  ~Tile();

  T* GetAtom(Point<int>* pt);

  T* GetAtom(int x, int y);

  T* GetAtom(int i);

  EventWindow* CreateWindow(Point<int>* ctr);

  EventWindow* CreateRandomWindow();

};

#include "tile.tcc"

#endif /*TILE_H*/
