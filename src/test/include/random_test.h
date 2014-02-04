#ifndef RANDOM_TEST_H      /* -*- C++ -*- */
#define RANDOM_TEST_H

#include "random.h"

namespace MFM {
  class RandomTest
  {
  private:
    static Random & setup();
    static void Test_randomSetSeed();
    static void Test_randomDeterministics();

  public:
    static void Test_RunTests();
  };
}
#endif /*RANDOM_TEST_H*/
