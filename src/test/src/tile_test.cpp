#include "assert.h"
#include "Tile.h"
#include "P1Atom.h"
#include "Point.h"
#include "tile_test.h"
#include "Element_Res.h"

namespace MFM {

void TileTest::Test_tilePlaceAtom()
{
  TestTile tile;
  tile.RegisterElement(Element_Res<TestCoreConfig>::THE_INSTANCE);

  TestAtom atom(Element_Res<TestCoreConfig>::THE_INSTANCE.GetDefaultAtom());
  SPoint loc(10, 10);

  tile.PlaceAtom(atom, loc);

  TestAtom other = *tile.GetAtom(loc);

  assert(other.GetType() == atom.GetType());
}
} /* namespace MFM */

