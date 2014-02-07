#include "assert.h"
#include "tile.h"
#include "p1atom.h"
#include "point.h"
#include "tile_test.h"

namespace MFM {

void TileTest::Test_tilePlaceAtom()
{
  TileP1Atom tile;
  P1Atom atom(10);
  SPoint loc(10, 10);

  tile.PlaceAtom(atom, loc);

  P1Atom other = *tile.GetAtom(loc);

  assert(other.GetState() == atom.GetState());
}
} /* namespace MFM */

