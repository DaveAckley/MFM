#include "ElementTable.h"
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "assert.h"
#include "EventWindow_Test.h"
#include "EventWindow.h"
#include "P1Atom.h"
#include "Point.h"

namespace MFM {

void EventWindow_Test::Test_eventwindowConstruction()
{
  TestTile tile;
  Element_Dreg<TestCoreConfig>::THE_INSTANCE.AllocateType();
  tile.RegisterElement(Element_Dreg<TestCoreConfig>::THE_INSTANCE);

  //  P1Atom * atoms = tile.GetAtoms();
  Point<s32> center(4, 4);
  Point<s32> zero(0, 0);
  const u32 DREG_TYPE = Element_Dreg<TestCoreConfig>::THE_INSTANCE.GetType();

  tile.PlaceAtom(TestAtom(DREG_TYPE,0,0,0), center);

  TestEventWindow ew(tile);
  ew.SetCenterInTile(center);

  TestAtom catom = ew.GetCenterAtom();

  assert(catom.GetType() == DREG_TYPE);

  catom = ew.GetRelativeAtom(zero);

  assert(catom.GetType() == DREG_TYPE);
}

void EventWindow_Test::Test_eventwindowWrite()
{
  TestTile tile;
  Element_Dreg<TestCoreConfig>::THE_INSTANCE.AllocateType();
  Element_Res<TestCoreConfig>::THE_INSTANCE.AllocateType();
  tile.RegisterElement(Element_Dreg<TestCoreConfig>::THE_INSTANCE);
  tile.RegisterElement(Element_Res<TestCoreConfig>::THE_INSTANCE);

  // We must ensure neither center nor 'absolute' ends up hitting the
  // cache!  Our TestTile has no connections, so Tile::PlaceAtom
  // ignores all cache write attempts to it!
  SPoint center(8, 8);
  SPoint absolute(-2, 0);
  SPoint zero(0, 0);
  const u32 DREG_TYPE = Element_Dreg<TestCoreConfig>::THE_INSTANCE.GetType();
  const u32 RES_TYPE = Element_Res<TestCoreConfig>::THE_INSTANCE.GetType();

  const u32 EMPTY_TYPE = Element_Empty<TestCoreConfig>::THE_INSTANCE.GetType();

  absolute.Add(center);

  const TestAtom * erased1 = tile.GetAtom(center);
  const TestAtom * erased2 = tile.GetAtom(absolute);

  absolute.Subtract(center);

  assert(erased1->GetType() == EMPTY_TYPE);
  assert(erased2->GetType() == EMPTY_TYPE);

  TestEventWindow ew(tile);

  ew.SetCenterInTile(center);

  ew.SetRelativeAtom(zero, TestAtom(DREG_TYPE,0,0,0));
  ew.SetRelativeAtom(absolute, TestAtom(RES_TYPE,0,0,0));

  assert(erased1->GetType() == DREG_TYPE);
  assert(erased2->GetType() == RES_TYPE);

}

} /* namespace MFM */
