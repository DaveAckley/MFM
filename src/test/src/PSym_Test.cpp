#include "assert.h"
#include "PSym_Test.h"
#include "itype.h"

namespace MFM {

  void PSym_Test::Test_RunTests() {
    Test_PSymTemplates();
    Test_PSymMap();
    Test_PSymInverse();
  }

  void PSym_Test::Test_PSymTemplates()
  {
    const SPoint in(3,5);

    // The eight symmetries map as expected
    assert(SymMap<PSYM_DEG000L>(in) == SPoint( 3, 5));
    assert(SymMap<PSYM_DEG090L>(in) == SPoint(-5, 3));
    assert(SymMap<PSYM_DEG180L>(in) == SPoint(-3,-5));
    assert(SymMap<PSYM_DEG270L>(in) == SPoint( 5,-3));
    assert(SymMap<PSYM_DEG000R>(in) == SPoint( 3,-5));
    assert(SymMap<PSYM_DEG090R>(in) == SPoint( 5, 3));
    assert(SymMap<PSYM_DEG180R>(in) == SPoint(-3, 5));
    assert(SymMap<PSYM_DEG270R>(in) == SPoint(-5,-3));

    // Alternate symmetry names act the same as the canonical
    assert(SymMap<PSYM_FLIPX>(in) == SymMap<PSYM_DEG180R>(in));
    assert(SymMap<PSYM_FLIPY>(in) == SymMap<PSYM_DEG000R>(in));
    assert(SymMap<PSYM_FLIPXY>(in)== SymMap<PSYM_DEG180L>(in));

  }

  void PSym_Test::Test_PSymMap()
  {
    const SPoint in(1,2);
    const SPoint err(9,10);

    // The eight symmetries map as expected
    assert(SymMap(in, PSYM_DEG000L, err) == SPoint( 1, 2));
    assert(SymMap(in, PSYM_DEG090L, err) == SPoint(-2, 1));
    assert(SymMap(in, PSYM_DEG180L, err) == SPoint(-1,-2));
    assert(SymMap(in, PSYM_DEG270L, err) == SPoint( 2,-1));
    assert(SymMap(in, PSYM_DEG000R, err) == SPoint( 1,-2));
    assert(SymMap(in, PSYM_DEG090R, err) == SPoint( 2, 1));
    assert(SymMap(in, PSYM_DEG180R, err) == SPoint(-1, 2));
    assert(SymMap(in, PSYM_DEG270R, err) == SPoint(-2,-1));

    // Point symmetries don't move the origin
    const SPoint o(0,0);
    assert(SymMap(o, PSYM_DEG000L, err) == o);
    assert(SymMap(o, PSYM_DEG090L, err) == o);
    assert(SymMap(o, PSYM_DEG180L, err) == o);
    assert(SymMap(o, PSYM_DEG270L, err) == o);
    assert(SymMap(o, PSYM_DEG000R, err) == o);
    assert(SymMap(o, PSYM_DEG090R, err) == o);
    assert(SymMap(o, PSYM_DEG180R, err) == o);
    assert(SymMap(o, PSYM_DEG270R, err) == o);

    // Error cases report given error
    assert(SymMap(in, (PointSymmetry) -1, err) == err);
    assert(SymMap(in, (PointSymmetry) 1000, err) == err);

    // Which is not always the same
    const SPoint err2(90,100);
    assert(SymMap(in, (PointSymmetry) -1, err) != err2);
    assert(SymMap(in, (PointSymmetry) 1000, err) != err2);

    // Alternate symmetry names act the same as the canonical
    assert(SymMap(in,PSYM_FLIPX,err) == SymMap(in, PSYM_DEG180R,err2));
    assert(SymMap(in,PSYM_FLIPY,err) == SymMap(in, PSYM_DEG000R,err2));
    assert(SymMap(in,PSYM_FLIPXY,err)== SymMap(in, PSYM_DEG180L,err2));
  }

  void PSym_Test::Test_PSymInverse()
  {
    const unsigned num = 4;
    const SPoint in[num] = {SPoint(-1, 0), SPoint(0, -1), SPoint(0, 1), SPoint(1, 0)};
    const SPoint err(10, 10);

    // Inverse of symmetry should return the original point
    for (unsigned i = 0; i < num; i++) {
      assert(SymMap(SymMap(in[i], PSYM_DEG000L, err), SymInverse(PSYM_DEG000L), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG090L, err), SymInverse(PSYM_DEG090L), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG180L, err), SymInverse(PSYM_DEG180L), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG270L, err), SymInverse(PSYM_DEG270L), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG000R, err), SymInverse(PSYM_DEG000R), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG090R, err), SymInverse(PSYM_DEG090R), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG180R, err), SymInverse(PSYM_DEG180R), err) == in[i]);
      assert(SymMap(SymMap(in[i], PSYM_DEG270R, err), SymInverse(PSYM_DEG270R), err) == in[i]);
    }
  }

} /* namespace MFM */
