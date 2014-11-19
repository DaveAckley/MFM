#include "assert.h"
#include "PSym_Test.h"
#include "itype.h"

namespace MFM {

  void PSym_Test::Test_RunTests() {
    Test_PSymTemplates();
    Test_PSymMap();
  }

  void PSym_Test::Test_PSymTemplates()
  {
    const SPoint in(3,5);

    // The eight symmetries map as expected
    assert(Map<PSYM_DEG000L>(in) == SPoint( 3, 5));
    assert(Map<PSYM_DEG090L>(in) == SPoint(-5, 3));
    assert(Map<PSYM_DEG180L>(in) == SPoint(-3,-5));
    assert(Map<PSYM_DEG270L>(in) == SPoint( 5,-3));
    assert(Map<PSYM_DEG000R>(in) == SPoint( 3,-5));
    assert(Map<PSYM_DEG090R>(in) == SPoint( 5, 3));
    assert(Map<PSYM_DEG180R>(in) == SPoint(-3, 5));
    assert(Map<PSYM_DEG270R>(in) == SPoint(-5,-3));

    // Alternate symmetry names act the same as the canonical
    assert(Map<PSYM_FLIPX>(in) == Map<PSYM_DEG180R>(in));
    assert(Map<PSYM_FLIPY>(in) == Map<PSYM_DEG000R>(in));
    assert(Map<PSYM_FLIPXY>(in)== Map<PSYM_DEG180L>(in));

  }

  void PSym_Test::Test_PSymMap()
  {
    const SPoint in(1,2);
    const SPoint err(9,10);

    // The eight symmetries map as expected
    assert(Map(in, PSYM_DEG000L, err) == SPoint( 1, 2));
    assert(Map(in, PSYM_DEG090L, err) == SPoint(-2, 1));
    assert(Map(in, PSYM_DEG180L, err) == SPoint(-1,-2));
    assert(Map(in, PSYM_DEG270L, err) == SPoint( 2,-1));
    assert(Map(in, PSYM_DEG000R, err) == SPoint( 1,-2));
    assert(Map(in, PSYM_DEG090R, err) == SPoint( 2, 1));
    assert(Map(in, PSYM_DEG180R, err) == SPoint(-1, 2));
    assert(Map(in, PSYM_DEG270R, err) == SPoint(-2,-1));

    // Point symmetries don't move the origin
    const SPoint o(0,0);
    assert(Map(o, PSYM_DEG000L, err) == o);
    assert(Map(o, PSYM_DEG090L, err) == o);
    assert(Map(o, PSYM_DEG180L, err) == o);
    assert(Map(o, PSYM_DEG270L, err) == o);
    assert(Map(o, PSYM_DEG000R, err) == o);
    assert(Map(o, PSYM_DEG090R, err) == o);
    assert(Map(o, PSYM_DEG180R, err) == o);
    assert(Map(o, PSYM_DEG270R, err) == o);

    // Error cases report given error
    assert(Map(in, (PointSymmetry) -1, err) == err);
    assert(Map(in, (PointSymmetry) 1000, err) == err);

    // Which is not always the same
    const SPoint err2(90,100);
    assert(Map(in, (PointSymmetry) -1, err) != err2);
    assert(Map(in, (PointSymmetry) 1000, err) != err2);

    // Alternate symmetry names act the same as the canonical
    assert(Map(in,PSYM_FLIPX,err) == Map(in, PSYM_DEG180R,err2));
    assert(Map(in,PSYM_FLIPY,err) == Map(in, PSYM_DEG000R,err2));
    assert(Map(in,PSYM_FLIPXY,err)== Map(in, PSYM_DEG180L,err2));

  }

} /* namespace MFM */
