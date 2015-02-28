#include "assert.h"
#include "Tile.h"
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
    TestTile tile;
    u32 W = tile.TILE_SIDE;
    u32 dist;

    dist = tile.GetSquareDistanceFromCenter(SPoint(W / 2, W / 2));
    assert(dist==1);

    dist = tile.GetSquareDistanceFromCenter(SPoint(0, 0));
    assert(dist==20);

    dist = tile.GetSquareDistanceFromCenter(SPoint(0, W / 2));
    assert(dist==20);

    dist = tile.GetSquareDistanceFromCenter(SPoint(W / 2, 0));
    assert(dist==20);

    dist = tile.GetSquareDistanceFromCenter(SPoint(W -1  , W - 1));
    assert(dist==20);

    dist = tile.GetSquareDistanceFromCenter(SPoint(W, W));
    assert(dist==21);

    dist = tile.GetSquareDistanceFromCenter(SPoint(-100, 50));
    assert(dist==120);

  }

  void Tile_Test::Test_tilePlaceAtom()
  {
    TestTile tile;
    ElementTypeNumberMap<TestEventConfig> etnm;
    Element_Res<TestEventConfig>::THE_INSTANCE.AllocateType(etnm);
    tile.RegisterElement(Element_Res<TestEventConfig>::THE_INSTANCE);

    TestAtom atom(Element_Res<TestEventConfig>::THE_INSTANCE.GetDefaultAtom());
    SPoint loc(10, 10);

    tile.PlaceAtom(atom, loc);

    TestAtom other = *tile.GetAtom(loc);

    assert(other.GetType() == atom.GetType());
  }
} /* namespace MFM */
