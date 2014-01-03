#include "elementtable.hpp"
#include "assert.h"
#include "eventwindow_test.hpp"
#include "eventwindow.h"
#include "p1atom.h"
#include "point.h"

void EventWindowTest::Test_eventwindowConstruction()
{ 
  P1Atom atoms[64];
  Point<int> center(4, 4);
  Point<int> zero(0, 0);

  atoms[center.GetX() + center.GetY() * 8].SetState(ELEMENT_DREG);

  EventWindow<P1Atom> ew(center, atoms, 8);

  P1Atom catom = ew.GetCenterAtom();

  assert(catom.GetState() == ELEMENT_DREG);

  catom = ew.GetRelativeAtom(zero);

  assert(catom.GetState() == ELEMENT_DREG);
  
}

void EventWindowTest::Test_eventwindowWrite()
{
  
}
