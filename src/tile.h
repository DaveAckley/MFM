#ifndef TILE_H
#define TILE_H

#include "point.h"
#include "eventwindow.h"

/* The length, in sites, of a Tile.*/
#define TILE_WIDTH 16

/*The number of sites a tile contains.*/
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)

class EventWindow;
class Tile
{
private:
  Atom* m_atoms[TILE_SIZE];

public:

  Tile();

  ~Tile();

  Atom* GetAtom(Point<int>* pt);

  Atom* GetAtom(int x, int y);

  Atom* GetAtom(int i);

  EventWindow* CreateWindow(Point<int>* ctr);

  EventWindow* CreateRandomWindow();

};

#endif /*TILE_H*/
