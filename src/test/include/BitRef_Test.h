#ifndef BITREF_TEST_H      /* -*- C++ -*- */
#define BITREF_TEST_H

#include "Test_Common.h"
#include "UlamRef.h"

namespace MFM {

  class BitRef_Test
  {
  private:
    static TestAtom setup();

  public:
    static void Test_RunTests();

    static void Test_BitRefCtors();

    static void Test_BitRefRead();

    static void Test_BitRefReadLong();

    static void Test_BitRefWrite();

    static void Test_BitRefWriteLong();

  };
} /* namespace MFM */
#endif /*BITREF_TEST_H*/
