#include "assert.h"
#include "grid.h"
#include "p1atom.h"
#include "grid_test.h"

namespace MFM {

void GridTest::Test_gridPlaceAtom()
{
  
  P1Atom atom(23);
  
  ElementTableP1Atom table(&P1Atom::StateFunc);

  GridP1Atom grid(2,2, &table);

  SPoint gloc(5, 10);

  grid.PlaceAtom(atom, gloc);

  P1Atom* out = grid.GetAtom(gloc);

  assert(out->GetState() == atom.GetState());
  
}
} /* namespace MFM */

