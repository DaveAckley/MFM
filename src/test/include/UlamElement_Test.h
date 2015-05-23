#ifndef ULAMELEMENT_TEST_H      /* -*- C++ -*- */
#define ULAMELEMENT_TEST_H

#include "UlamElement.h"

namespace MFM {

  class UlamElement_Test
  {
  private:

  public:
    static void Test_RunTests();

    static void Test_UlamElementCtors();

    static void Test_UlamElementAllocate();

    static void Test_UlamElementRead();

    static void Test_UlamElementSize();

    static void Test_UlamElementWrite();

    static void Test_UlamElementSplitWrites();

    static void Test_UlamElementSetAndClearBits();

    static void Test_UlamElementStoreBits();

    static void Test_UlamElementLong();

  };
} /* namespace MFM */
#endif /*ULAMELEMENT_TEST_H*/
