#ifndef FXP_TEST_H      /* -*- C++ -*- */
#define FXP_TEST_H

#include "FXP.h"

namespace MFM {

  class FXP_Test
  {
  private:
    template <int p>
      static void assertTolerance(const char * file, const int lineno, const int ulp, FXP<p> num, double toThis);

    static void Test_FXPCtors();
    static void Test_FXPOps();
    static void Test_FXPCloser();
    static void Test_FXPFarther();

  public:
    static void Test_RunTests();
  };

} /* namespace MFM */
#endif /*FXP_TEST_H*/
