#include "ElementTable.h"
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "assert.h"
#include "EventWindow_Test.h"
#include "EventWindow.h"
#include "P1Atom.h"
#include "Point.h"

namespace MFM {

void EventWindowTest::Test_eventwindowConstruction()
{
  TestTile tile;
  tile.RegisterElement(Element_Dreg<TestCoreConfig>::THE_INSTANCE);

  //  P1Atom * atoms = tile.GetAtoms();
  Point<s32> center(4, 4);
  Point<s32> zero(0, 0);
  const u32 DREG_TYPE = Element_Dreg<TestCoreConfig>::TYPE;

  tile.PlaceAtom(TestAtom(DREG_TYPE,0,0,0), center);

  TestEventWindow ew(tile);
  ew.SetCenterInTile(center);

  TestAtom catom = ew.GetCenterAtom();

  assert(catom.GetType() == DREG_TYPE);

  catom = ew.GetRelativeAtom(zero);

  assert(catom.GetType() == DREG_TYPE);
}

void EventWindowTest::Test_eventwindowWrite()
{
  TestTile tile;
  tile.RegisterElement(Element_Dreg<TestCoreConfig>::THE_INSTANCE);
  tile.RegisterElement(Element_Res<TestCoreConfig>::THE_INSTANCE);

  SPoint center(4, 4);
  SPoint absolute(-2, 0);
  SPoint zero(0, 0);
  const u32 DREG_TYPE = Element_Dreg<TestCoreConfig>::TYPE;
  const u32 RES_TYPE = Element_Res<TestCoreConfig>::TYPE;

  absolute.Add(center);

  const TestAtom * erased1 = tile.GetAtom(center);
  const TestAtom * erased2 = tile.GetAtom(absolute);

  absolute.Subtract(center);

  assert(erased1->GetType() == 0);
  assert(erased2->GetType() == 0);

  TestEventWindow ew(tile);

  ew.SetCenterInTile(center);

  ew.SetRelativeAtom(zero, TestAtom(DREG_TYPE,0,0,0));
  ew.SetRelativeAtom(absolute, TestAtom(RES_TYPE,0,0,0));

  assert(erased1->GetType() == DREG_TYPE);
  assert(erased2->GetType() == RES_TYPE);

}

} /* namespace MFM */