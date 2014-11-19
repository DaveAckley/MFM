#include "assert.h"
#include "Tile.h"
#include "P1Atom.h"
#include "Point.h"
#include "Tile_Test.h"
#include "Element_Res.h"

namespace MFM {

  void Tile_Test::Test_RunTests() {
    Test_tileSquareDistances();
    Test_tilePlaceAtom();
  }

  void Tile_Test::Test_tileSquareDistances()
  {
    u32 W = TestTile::TILE_WIDTH;
    u32 dist;

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(W / 2, W / 2));
    assert(dist==1);

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(0, 0));
    assert(dist==20);

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(0, W / 2));
    assert(dist==20);

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(W / 2, 0));
    assert(dist==20);

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(W -1  , W - 1));
    assert(dist==20);

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(W, W));
    assert(dist==21);

    dist = TestTile::GetSquareDistanceFromCenter(SPoint(-100, 50));
    assert(dist==120);

  }

  void Tile_Test::Test_tilePlaceAtom()
  {
    TestTile tile;
    Element_Res<TestCoreConfig>::THE_INSTANCE.AllocateType();
    tile.RegisterElement(Element_Res<TestCoreConfig>::THE_INSTANCE);

    TestAtom atom(Element_Res<TestCoreConfig>::THE_INSTANCE.GetDefaultAtom());
    SPoint loc(10, 10);

    tile.PlaceAtom(atom, loc);

    TestAtom other = *tile.GetAtom(loc);

    assert(other.GetType() == atom.GetType());
  }
} /* namespace MFM */
