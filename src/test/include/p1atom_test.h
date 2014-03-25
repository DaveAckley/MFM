#ifndef P1ATOM_TEST_H      /* -*- C++ -*- */
#define P1ATOM_TEST_H

#include "test_common.h"

namespace MFM {

class P1AtomTest
{ 
public:
  static void Test_p1atomState();

  static void Test_p1atomLBCount();

  static void Test_p1atomSBCount();

  static void Test_p1atomReadBody();

  static void Test_p1atomAddLB();

  static void Test_p1atomAddSB();

  static void Test_p1atomFillSB();

  static void Test_p1atomFillLB();

  static void Test_p1atomRemoveLB();

  static void Test_p1atomRemoveSB();
};
} /* namespace MFM */
#endif /*P1ATOM_TEST_H*/

