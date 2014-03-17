#include "assert.h"
#include "grid.h"
#include "P1Atom.h"
#include "grid_test.h"
#include "Element_Res.h"

namespace MFM {

void GridTest::Test_gridPlaceAtom()
{
  
  GridP1Atom grid;

  grid.SetSeed(1);
  grid.Reinit();

  grid.Needed(Element_Res<P1Atom,4>::THE_INSTANCE);

  P1Atom atom(Element_Res<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());

  SPoint gloc(5, 10);

  grid.PlaceAtom(atom, gloc);

  const P1Atom* out = grid.GetAtom(gloc);

  assert(out->GetType() == atom.GetType());
  
}
} /* namespace MFM */

