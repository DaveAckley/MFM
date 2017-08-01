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
    TestTile tile(TILE_STAGGER_NONE);
    u32 W = tile.TILE_WIDTH;
    u32 H = tile.TILE_HEIGHT;
    u32 dist;

    dist = tile.GetSquareDistanceFromEdge(SPoint(W / 2, H / 2));
    assert(dist==20);

    dist = tile.GetSquareDistanceFromEdge(SPoint(0, 0));
    assert(dist==0);

    dist = tile.GetSquareDistanceFromEdge(SPoint(0, H / 2));
    assert(dist==H/2);

    dist = tile.GetSquareDistanceFromEdge(SPoint(W / 2, 0));
    assert(dist==W/2);

    dist = tile.GetSquareDistanceFromEdge(SPoint(W -1  , H - 1));
    assert(dist==1);

    dist = tile.GetSquareDistanceFromEdge(SPoint(1, 1));
    assert(dist==1);
  }

  void Tile_Test::Test_tilePlaceAtom()
  {
    TestTile tile(TILE_STAGGER_NONE);
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
