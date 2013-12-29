#include "assert.h"
#include "grid.h"
#include "p1atom.h"
#include "grid_test.hpp"

void GridTest::Test_gridPlaceAtom()
{
  P1Atom atom(23);
  
  ElementTable<P1Atom> table(&P1Atom::StateFunc);

  Grid<P1Atom> grid(2,2, &table);

  Point<int> gloc(5, 10);

  grid.PlaceAtom(atom, gloc);

  P1Atom* out = grid.GetAtom(gloc);

  assert(out->GetState() == atom.GetState());
}
