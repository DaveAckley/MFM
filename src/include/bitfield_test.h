#ifndef BITFIELD_TEST_H      /* -*- C++ -*- */
#define BITFIELD_TEST_H

#include "bitfield.h"

class BitFieldTest
{
private:
  static BitField<256>* setup();

public:
  static void Test_bitfieldAllocate();

  static void Test_bitfieldRead();
  
  static void Test_bitfieldSize();

  static void Test_bitfieldWrite();

  static void Test_bitfieldInsert();

  static void Test_bitfieldRemove();
};

#endif /*BITFIELD_TEST_H*/
