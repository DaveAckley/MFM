#ifndef ULAMREF_TEST_H      /* -*- C++ -*- */
#define ULAMREF_TEST_H

#include "Test_Common.h"
#include "UlamRef.h"

namespace MFM {

  class UlamRef_Test
  {
  private:
    static TestAtom setup();

  public:
    static void Test_RunTests();

    static void Test_UlamRefCtors();

    static void Test_UlamRefRead();

    static void Test_UlamRefReadLong();

    static void Test_UlamRefWrite();

    static void Test_UlamRefWriteLong();

  };
} /* namespace MFM */
#endif /*ULAMREF_TEST_H*/
