#include "assert.h"
#include "Grid.h"
#include "P1Atom.h"
#include "Grid_Test.h"
#include "Element_Res.h"

namespace MFM {

void GridTest::Test_gridPlaceAtom()
{
  
  TestGrid grid;

  grid.SetSeed(1);
  grid.Reinit();

  grid.Needed(Element_Res<TestCoreConfig>::THE_INSTANCE);

  TestAtom atom(Element_Res<TestCoreConfig>::THE_INSTANCE.GetDefaultAtom());

  SPoint gloc(5, 10);

  grid.PlaceAtom(atom, gloc);

  const TestAtom* out = grid.GetAtom(gloc);

  assert(out->GetType() == atom.GetType());
  
}
} /* namespace MFM */

