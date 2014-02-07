#include "assert.h"
#include "bitfield_test.h"
#include "itype.h"

namespace MFM {

static BitField<256> bits;

BitField<256>* BitFieldTest::setup()
{
  u32 vals[8] =
  {
    0xffffffff,
    0x00000000,
    0x12345678,
    0x9abcdef0,
    0x0fedcba9,
    0x87654321,
    0x00000000,
    0xffffffff
  };

  bits = BitField<256>(vals);

  return &bits;
}

void BitFieldTest::Test_bitfieldAllocate()
{
  BitField<256>* bits = setup();

  assert(bits->Read(0, 32) == 0xffffffff);
}

void BitFieldTest::Test_bitfieldRead()
{
  BitField<256>* bits = setup();

  assert(bits->Read(16, 32) == 0xffff0000);
  assert(bits->Read(64, 32) == 0x12345678);
  assert(bits->Read(56, 16) == 0x00000012);
}

void BitFieldTest::Test_bitfieldSize()
{
  BitField<64> bits;
  BitField<1024> bigBits;
  
  assert(sizeof(bits) * 8 == 64);
  assert(sizeof(bigBits) * 8 == 1024);
}

void BitFieldTest::Test_bitfieldWrite()
{
  BitField<256>* bits = setup();

  bits->Write(16, 32, 0xa0a0b0b0);

  assert(bits->Read(0, 32) == 0xffffa0a0);
  assert(bits->Read(32, 32) == 0xb0b00000);
  assert(bits->Read(224, 32) == 0xffffffff);
}

void BitFieldTest::Test_bitfieldInsert()
{
  BitField<256>* bits = setup();

  bits->Insert(8, 16, 0x1234);

  assert(bits->Read(0, 32) == 0xff1234ff);
  assert(bits->Read(32, 32) == 0xffff0000);
  assert(bits->Read(224, 32) == 0x0000ffff);
}

void BitFieldTest::Test_bitfieldRemove()
{
  BitField<256>* bits = setup();

  bits->Remove(0, 32);
  
  assert(bits->Read(0  , 32) == 0x00000000);
  assert(bits->Read(32 , 32) == 0x12345678);
  assert(bits->Read(224, 32) == 0x00000000);
}
} /* namespace MFM */

