#ifndef PSYM_TEST_H      /* -*- C++ -*- */
#define PSYM_TEST_H

#include "PSym.h"

namespace MFM {

  class PSym_Test
  {
  private:

  public:
    static void Test_RunTests();

    static void Test_PSymMap();

    static void Test_PSymTemplates();

    static void Test_PSymInverse();
  };
} /* namespace MFM */
#endif /*PSYM_TEST_H*/
