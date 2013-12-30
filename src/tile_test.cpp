#include "assert.h"
#include "tile.h"
#include "p1atom.h"
#include "point.h"
#include "tile_test.hpp"

void TileTest::Test_tilePlaceAtom()
{
  Tile<P1Atom> tile;
  P1Atom atom(10);
  Point<int> loc(10, 10);

  tile.PlaceAtom(atom, loc);

  P1Atom other = *tile.GetAtom(&loc);

  assert(other.GetState() == atom.GetState());
}
