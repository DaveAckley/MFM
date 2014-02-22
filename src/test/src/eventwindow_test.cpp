#include "elementtable.h"
#include "element_dreg.h"
#include "element_res.h"
#include "assert.h"
#include "eventwindow_test.h"
#include "eventwindow.h"
#include "p1atom.h"
#include "point.h"

namespace MFM {

void EventWindowTest::Test_eventwindowConstruction()
{ 
  TileP1Atom tile;
  tile.RegisterElement(Element_Dreg<P1Atom,4>::THE_INSTANCE);

  //  P1Atom * atoms = tile.GetAtoms();
  Point<s32> center(4, 4);
  Point<s32> zero(0, 0);
  const u32 DREG_TYPE = Element_Dreg<P1Atom,4>::TYPE;

  tile.PlaceAtom(P1Atom(DREG_TYPE,0,0,0), center);

  EventWindowP1Atom ew(tile, 8, 0xff);
  ew.SetCenter(center);

  P1Atom catom = ew.GetCenterAtom();

  assert(catom.GetType() == DREG_TYPE);

  catom = ew.GetRelativeAtom(zero);

  assert(catom.GetType() == DREG_TYPE);
  
}

void EventWindowTest::Test_eventwindowWrite()
{
  TileP1Atom tile;
  tile.RegisterElement(Element_Dreg<P1Atom,4>::THE_INSTANCE);
  tile.RegisterElement(Element_Res<P1Atom,4>::THE_INSTANCE);

  SPoint center(4, 4);
  SPoint absolute(-2, 0);
  SPoint zero(0, 0);
  const u32 DREG_TYPE = Element_Dreg<P1Atom,4>::TYPE;
  const u32 RES_TYPE = Element_Res<P1Atom,4>::TYPE;

  absolute.Add(center);

  P1Atom * erased1 = tile.GetAtom(center);
  P1Atom * erased2 = tile.GetAtom(absolute);

  absolute.Subtract(center);

  assert(erased1->GetType() == 0);
  assert(erased2->GetType() == 0);

  EventWindowP1Atom ew(tile, 8, 0xff);

  ew.SetCenter(center);

  ew.SetRelativeAtom(zero, P1Atom(DREG_TYPE,0,0,0));
  ew.SetRelativeAtom(absolute, P1Atom(RES_TYPE,0,0,0));

  assert(erased1->GetType() == DREG_TYPE);
  assert(erased2->GetType() == RES_TYPE);

}

} /* namespace MFM */

