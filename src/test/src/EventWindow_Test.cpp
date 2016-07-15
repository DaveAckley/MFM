#include "ElementTable.h"
#include "Element_Wall.h"
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "assert.h"
#include "EventWindow_Test.h"
#include "EventWindow.h"
#include "Point.h"

namespace MFM {

  void EventWindow_Test::Test_RunTests()
  {
    Test_EventWindowConstruction();
    Test_EventWindowNoLockOpen();
    Test_EventWindowWrite();
  }

  void EventWindow_Test::Test_EventWindowConstruction()
  {
    TestTile tile;
    TestEventWindow ew(tile);
  }

  void EventWindow_Test::Test_EventWindowNoLockOpen()
  {
    TestTile tile;
    ElementTypeNumberMap<TestEventConfig> etnm;
    Element_Wall<TestEventConfig>::THE_INSTANCE.AllocateTypeForTesting(etnm); // Need something non-diffusing..
    tile.RegisterElement(Element_Wall<TestEventConfig>::THE_INSTANCE);

    SPoint center(15, 20);  // Hitting no caches, for starters
    SPoint zero(0, 0);
    const u32 WALL_TYPE = Element_Wall<TestEventConfig>::THE_INSTANCE.GetType();

    tile.PlaceAtom(TestAtom(WALL_TYPE,0,0,0), center);

    TestEventWindow & ew = tile.GetEventWindow();

    ew.SetEventWindowsExecuted(1000000); // make event 0 look very old to avoid recency reject

    bool success = ew.TryEventAt(center);
    assert(success);

    TestAtom catom = ew.GetCenterAtomDirect();

    assert(catom.GetType() == WALL_TYPE);

    catom = ew.GetRelativeAtomDirect(zero);

    assert(catom.GetType() == WALL_TYPE);

  }

  void EventWindow_Test::Test_EventWindowWrite()
  {
    TestTile tile;
    ElementTypeNumberMap<TestEventConfig> etnm;
    Element_Dreg<TestEventConfig>::THE_INSTANCE.AllocateTypeForTesting(etnm);
    Element_Res<TestEventConfig>::THE_INSTANCE.AllocateTypeForTesting(etnm);
    Element_Wall<TestEventConfig>::THE_INSTANCE.AllocateTypeForTesting(etnm);
    tile.RegisterElement(Element_Dreg<TestEventConfig>::THE_INSTANCE);
    tile.RegisterElement(Element_Res<TestEventConfig>::THE_INSTANCE);
    tile.RegisterElement(Element_Wall<TestEventConfig>::THE_INSTANCE);

    // We must ensure neither center nor 'absolute' ends up hitting the
    // cache!  Our TestTile has no connections, so Tile::PlaceAtom
    // ignores all cache write attempts to it!
    SPoint center(8, 8);
    SPoint absolute(-1, 0);
    SPoint zero(0, 0);
    const u32 WALL_TYPE = Element_Wall<TestEventConfig>::THE_INSTANCE.GetType();
    const u32 DREG_TYPE = Element_Dreg<TestEventConfig>::THE_INSTANCE.GetType();
    const u32 RES_TYPE = Element_Res<TestEventConfig>::THE_INSTANCE.GetType();

    const u32 EMPTY_TYPE = Element_Empty<TestEventConfig>::THE_INSTANCE.GetType();

    absolute.Add(center);

    *(tile.GetWritableAtom(center)) = TestAtom(WALL_TYPE,0,0,0);

    const TestAtom * erased1 = tile.GetAtom(center);   // a Wall
    const TestAtom * erased2 = tile.GetAtom(absolute); // a Empty

    absolute.Subtract(center);

    assert(erased1->GetType() == WALL_TYPE);
    assert(erased2->GetType() == EMPTY_TYPE);

    TestEventWindow & ew = tile.GetEventWindow();
    ew.SetEventWindowsExecuted(1000000); // make event 0 look very old to avoid recency reject

    bool res = ew.TryEventAt(center);
    assert(res);

    ew.SetBoundary(4);
    ew.SetRelativeAtomDirect(zero, TestAtom(DREG_TYPE,0,0,0));
    ew.SetRelativeAtomDirect(absolute, TestAtom(RES_TYPE,0,0,0));

    assert(erased1->GetType() == WALL_TYPE);
    assert(erased2->GetType() == EMPTY_TYPE);

    ew.StoreToTile();

    assert(erased1->GetType() == DREG_TYPE);
    assert(erased2->GetType() == RES_TYPE);

  }

} /* namespace MFM */
