#ifndef FAIL_TEST_H      /* -*- C++ -*- */
#define FAIL_TEST_H

#include "Fail.h"

namespace MFM {

  class Fail_Test
  {
  private:

  public:
    static void Test_NoFail();
    static void Test_SimpleFail();
    static void Test_FailAndNot();
    static void Test_FailWithCode();
    static void Test_NestedFail();
    static void Test_UnsafeModification();

    static void Test_RunTests();
  };
} /* namespace MFM */
#endif /*FAIL_TEST_H*/
