#include "assert.h"
#include "Random_Test.h"
#include "itype.h"

namespace MFM {

  void Random_Test::Test_RunTests() {
    Test_randomSetSeed();
  }

  Random & Random_Test::setup()
  {
    static Random random(1);
    return random;
  }

  void Random_Test::Test_randomSetSeed()
  {
    Random & random = setup();
    const u32 NUMS = 100;
    u32 nums[NUMS];

    for (u32 i = 0; i < NUMS; ++i) {
      nums[i] = random.Create();
    }

    // Check that Random(1) matches Random()+setSeed(1)
    Random r2;
    r2.SetSeed(1);
    for (u32 i = 0; i < NUMS; ++i) {
      assert(nums[i]==r2.Create());
    }

    // Check that re-setSeedingRandom(1) also matches
    r2.SetSeed(1);
    for (u32 i = 0; i < NUMS; ++i) {
      assert(nums[i]==r2.Create());
    }

    // Check that setSeedingRandom(2) doesn't match
    u32 countSame = 0;
    r2.SetSeed(2);
    for (u32 i = 0; i < NUMS; ++i) {
      if (nums[i]==r2.Create()) ++countSame;
    }
    assert(countSame < NUMS);

  }

  void Random_Test::Test_randomDeterministics()
  {
    Random & random = setup();
    const u32 TRIES = 100;

    for (u32 i = 0; i < TRIES; ++i) {
      for (u32 max = 1; max < 100000; max *= 3)  {

        assert(random.Create(max) < max);

        assert(random.OddsOf(0,max)==false);
        assert(random.OddsOf(max,max)==true);
        assert(random.OddsOf(max+1,max)==true);

        s32 smax = (s32) max;
        s32 num = random.Between(-smax,smax);
        assert(num >= -smax);
        assert(num <= smax);
      }
    }
  }

} /* namespace MFM */
