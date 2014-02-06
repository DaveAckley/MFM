#include "assert.h"
#include "random_test.h"
#include "itype.h"

namespace MFM {
  void RandomTest::Test_RunTests() {
    Test_randomSetSeed();
  }

  Random & RandomTest::setup()
  {
    static Random random(1);
    return random;
  }

  void RandomTest::Test_randomSetSeed()
  {
    Random & random = setup();
    const u32 NUMS = 100;
    u32 nums[NUMS];

    for (u32 i = 0; i < NUMS; ++i) {
      nums[i] = random.create();
    }

    // Check that Random(1) matches Random()+setSeed(1)
    Random r2;
    r2.setSeed(1);
    for (u32 i = 0; i < NUMS; ++i) {
      assert(nums[i]==r2.create());
    }

    // Check that re-setSeedingRandom(1) also matches
    r2.setSeed(1);
    for (u32 i = 0; i < NUMS; ++i) {
      assert(nums[i]==r2.create());
    }

    // Check that setSeedingRandom(2) doesn't match
    u32 countSame = 0;
    r2.setSeed(2);
    for (u32 i = 0; i < NUMS; ++i) {
      if (nums[i]==r2.create()) ++countSame;
    }
    assert(countSame < NUMS);

  }

  void RandomTest::Test_randomDeterministics()
  {
    Random & random = setup();
    const u32 TRIES = 100;

    for (u32 i = 0; i < TRIES; ++i) {
      for (u32 max = 1; max < 100000; max *= 3)  {

        assert(random.create(max) < max);

        assert(random.oddsOf(0,max)==false);
        assert(random.oddsOf(max,max)==true);
        assert(random.oddsOf(max+1,max)==true);

        s32 smax = (s32) max;
        s32 num = random.between(-smax,smax);
        assert(num >= -smax);
        assert(num <= smax);
      }
    }
  }

}
