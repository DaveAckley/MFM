#include "assert.h"
#include "BitVector_test.h"
#include "itype.h"

namespace MFM {

  void BitVectorTest::Test_RunTests() {
    Test_bitVectorAllocate();
    Test_bitVectorRead();
    Test_bitVectorSize();
    Test_bitVectorWrite();
    Test_bitVectorSplitWrites();
  }

  static BitVector<256> bits;

  BitVector<256>* BitVectorTest::setup()
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

    bits = BitVector<256>(vals);

    return &bits;
  }

  void BitVectorTest::Test_bitVectorAllocate()
  {
    BitVector<256>* bits = setup();

    assert(bits->Read(0, 32) == 0xffffffff);
  }

  void BitVectorTest::Test_bitVectorRead()
  {
    BitVector<256>* bits = setup();

    assert(bits->Read(16, 32) == 0xffff0000);
    assert(bits->Read(64, 32) == 0x12345678);
    assert(bits->Read(56, 16) == 0x00000012);

    {
      BitVector<64> foo;
      assert(foo.Read(64-31, 31)==0);
    }
  }

  void BitVectorTest::Test_bitVectorSize()
  {
    BitVector<64> bits;
    BitVector<1024> bigBits;
  
    assert(sizeof(bits) * 8 == 64);
    assert(sizeof(bigBits) * 8 == 1024);
  }

  void BitVectorTest::Test_bitVectorWrite()
  {
    BitVector<256>* bits = setup();

    bits->Write(16, 32, 0xa0a0b0b0);

    assert(bits->Read(0, 32) == 0xffffa0a0);
    assert(bits->Read(32, 32) == 0xb0b00000);
    assert(bits->Read(224, 32) == 0xffffffff);
  }

  void BitVectorTest::Test_bitVectorSplitWrites()
  {
    BitVector<64> bits;

    // Nothing in there to start
    assert(bits.Read(0,32) == 0);
    assert(bits.Read(32,32) == 0);

    // Write four bits in bottom of 0 and 12 into top of 1
    bits.Write(28,16,0x1234);
    assert(bits.Read(0,32) == 0x1);
    assert(bits.Read(32,32) == 0x23400000);

  }

} /* namespace MFM */

