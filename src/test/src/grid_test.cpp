#include "assert.h"
#include "grid.h"
#include "p1atom.h"
#include "grid_test.h"

namespace MFM {

void GridTest::Test_gridPlaceAtom()
{
  
  GridP1Atom grid;

  grid.Needed(Element_Res<P1Atom,4>::THE_INSTANCE);

  P1Atom atom(Element_Res<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());

  SPoint gloc(5, 10);

  grid.PlaceAtom(atom, gloc);

  P1Atom* out = grid.GetAtom(gloc);

  assert(out->GetType() == atom.GetType());
  
}
} /* namespace MFM */
