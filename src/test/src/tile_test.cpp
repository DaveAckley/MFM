#include "assert.h"
#include "Tile.h"
#include "p1atom.h"
#include "point.h"
#include "tile_test.h"

namespace MFM {

void TileTest::Test_tilePlaceAtom()
{
  TileP1Atom tile;
  tile.RegisterElement(Element_Res<P1Atom,4>::THE_INSTANCE);

  P1Atom atom(Element_Res<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
  SPoint loc(10, 10);

  tile.PlaceAtom(atom, loc);

  P1Atom other = *tile.GetAtom(loc);

  assert(other.GetType() == atom.GetType());
}
} /* namespace MFM */

