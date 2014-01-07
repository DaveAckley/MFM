#include "elementtable.h"
#include "assert.h"
#include "eventwindow_test.h"
#include "eventwindow.h"
#include "p1atom.h"
#include "point.h"

void EventWindowTest::Test_eventwindowConstruction()
{ 
  P1Atom atoms[64];
  Point<int> center(4, 4);
  Point<int> zero(0, 0);

  atoms[center.GetX() + center.GetY() * 8].SetState(ELEMENT_DREG);

  EventWindowP1Atom ew(center, atoms, 8);

  P1Atom catom = ew.GetCenterAtom();

  assert(catom.GetState() == ELEMENT_DREG);

  catom = ew.GetRelativeAtom(zero);

  assert(catom.GetState() == ELEMENT_DREG);
  
}

void EventWindowTest::Test_eventwindowWrite()
{
  P1Atom atoms[64];
  Point<int> center(4, 4);
  Point<int> absolute(-2, 0);

  absolute.Add(center);

  atoms[center.GetX() + center.GetY() * 8].SetState(ELEMENT_DREG);
  atoms[absolute.GetX() + absolute.GetY() * 8].SetState(ELEMENT_RES);

  EventWindowP1Atom ew(center, atoms, 8);

  for(int i = 0; i < 64; i++)
  {
    atoms[i] = P1Atom(0);
  }

  P1Atom& erased1 = atoms[center.GetX() + center.GetY() * 8];
  P1Atom& erased2 = atoms[absolute.GetX() + absolute.GetY() * 8];

  assert(erased1.GetState() == 0);
  assert(erased2.GetState() == 0);

  ew.WriteTo(atoms, 8);

  P1Atom& reWritten1 = atoms[center.GetX() + center.GetY() * 8];
  P1Atom& reWritten2 = atoms[absolute.GetX() + absolute.GetY() * 8];

  assert(reWritten1.GetState() == ELEMENT_DREG);
  assert(reWritten2.GetState() == ELEMENT_RES);

  
}
