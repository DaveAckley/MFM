#include "assert.h"
#include "BitVector_Test.h"
#include "itype.h"

namespace MFM {

  const u32 vals[8] =
    {
      0x24681357, // 0   0
      0x11121314, // 1  32
      0x12345678, // 2  64
      0x9abcdef0, // 3  96
      0x0fedcba9, // 4 128
      0x87654321, // 5 160
      0x44332211, // 6 192
      0xfedbca09  // 7 224
    };

  void BitVector_Test::Test_RunTests() {
    Test_bitVectorLong();
    Test_bitVectorAllocate();
    Test_bitVectorRead();
    Test_bitVectorSize();
    Test_bitVectorWrite();
    Test_bitVectorSplitWrites();
    Test_bitVectorSetAndClearBits();
    Test_bitVectorStoreBits();
  }

  static BitVector<256> bits;

  BitVector<256>* BitVector_Test::setup()
  {

    bits = BitVector<256>(vals);

    return &bits;
  }

  void BitVector_Test::Test_bitVectorAllocate()
  {
    BitVector<256>* bits = setup();

    assert(bits->Read(0, 32) == 0x24681357);
  }

  void BitVector_Test::Test_bitVectorRead()
  {
    BitVector<256>* bits = setup();

    assert(bits->Read(16, 32) == 0x13571112);
    assert(bits->Read(64, 32) == 0x12345678);
    assert(bits->Read(56, 16) == 0x00001412);

    {
      BitVector<64> foo;
      assert(foo.Read(64-31, 31)==0);
    }
  }

  void BitVector_Test::Test_bitVectorSize()
  {
    BitVector<64> bits;
    BitVector<1024> bigBits;

    assert(sizeof(bits) * 8 == 64);
    assert(sizeof(bigBits) * 8 == 1024);
  }

  void BitVector_Test::Test_bitVectorWrite()
  {
    BitVector<256>* bits = setup();

    bits->Write(16, 32, 0xa0a0b0b0);

    assert(bits->Read(0, 32) == 0x2468a0a0);
    assert(bits->Read(32, 32) == 0xb0b01314);
    assert(bits->Read(224, 32) == 0xfedbca09);
  }

  void BitVector_Test::Test_bitVectorSplitWrites()
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

  static void assertUnchanged(u32 firstIdx, u32 lastIdx) {
    for (u32 i = firstIdx; i < lastIdx; ++i)
      assert(bits.Read(i*32,32) == vals[i]);
  }

  void BitVector_Test::Test_bitVectorSetAndClearBits()
  {
    BitVector<256>* bits;
    bits = setup();

    assertUnchanged(0,7);
    bits->ClearBits(0,64);

    assertUnchanged(2,7);
    assert(bits->Read(0,32) == 0);
    assert(bits->Read(32,32) == 0);

    bits->SetBits(48,64);  // hits 1,2,3
    assert(bits->Read(0,32) == 0);
    assert(bits->Read(32,32) == 0x0000ffff);
    assert(bits->Read(64,32) == 0xffffffff);
    assert(bits->Read(96,32) == 0xffffdef0);

    bits = setup();
    bits->ClearBits(31,33);  // 0,1; unaligned start, aligned end

    assert(bits->Read(0,32) == 0x24681356);
    assert(bits->Read(32,32) == 0x0);
    assertUnchanged(2,7);

    bits = setup();
    bits->ClearBits(128,32+32+32+1);  // 4-7; aligned start, unaligned end

    assertUnchanged(0,3);
    assert(bits->Read(128,32) == 0x0);
    assert(bits->Read(160,32) == 0x0);
    assert(bits->Read(192,32) == 0x0);
    assert(bits->Read(224,32) == 0x7edbca09);

    bits = setup();
    bits->SetBits(128,32+32+32+32);  // 4-6; aligned start, aligned end

    assertUnchanged(0,3);
    assert(bits->Read(128,32) == 0xffffffff);
    assert(bits->Read(160,32) == 0xffffffff);
    assert(bits->Read(192,32) == 0xffffffff);
    assertUnchanged(7,7);

    bits = setup();
    bits->SetBits(100,48);        // 3-4; unaligned start, unaligned end, adjacent

    assertUnchanged(0,2);
    assert(bits->Read( 96,32) == 0x9fffffff);
    assert(bits->Read(128,32) == 0xfffffba9);
    assertUnchanged(5,7);

    bits = setup();
    bits->ClearBits(4,48); // 0-1; unaligned start&end, adjacent, first unit

    assert(bits->Read(  0,32) == 0x20000000);
    assert(bits->Read( 32,32) == 0x00000314);
    assertUnchanged(2,7);

    bits = setup();
    bits->ClearBits(192+16,32); // 6-7; unaligned start&end, adjacent, last unit

    assertUnchanged(0,5);
    assert(bits->Read(192,32) == 0x44330000);
    assert(bits->Read(224,32) == 0x0000ca09);

    bits = setup();
    bits->ClearBits(32+16,32*5); // 1-6; unaligned start&end, non-adjacent

    assertUnchanged(0,0);
    assert(bits->Read( 32,32) == 0x11120000);
    assert(bits->Read( 64,32) == 0x00000000);
    assert(bits->Read( 96,32) == 0x00000000);
    assert(bits->Read(128,32) == 0x00000000);
    assert(bits->Read(160,32) == 0x00000000);
    assert(bits->Read(192,32) == 0x00002211);
    assertUnchanged(7,7);

    bits = setup();
    bits->ClearBits(16,32*7); // 0-7; unaligned start&end, all units

    assert(bits->Read(  0,32) == 0x24680000);
    assert(bits->Read( 32,32) == 0x00000000);
    assert(bits->Read( 64,32) == 0x00000000);
    assert(bits->Read( 96,32) == 0x00000000);
    assert(bits->Read(128,32) == 0x00000000);
    assert(bits->Read(160,32) == 0x00000000);
    assert(bits->Read(192,32) == 0x00000000);
    assert(bits->Read(224,32) == 0x0000ca09);

    bits = setup();
    bits->ClearBits(0,32*8); // 0-7; all bits cleared

    for (u32 i = 0; i < 32*8; i += 32)
      assert(bits->Read(i,32) == 0x00000000);

    bits->SetBits(0,32*8); // 0-7; all bits set

    for (u32 i = 0; i < 32*8; i += 32)
      assert(bits->Read(i,32) == 0xffffffff);
  }

  void BitVector_Test::Test_bitVectorStoreBits()
  {
    const u32 PATTERN = 0xdeadbeef;

    BitVector<256>* bits;
    bits = setup();
    bits->StoreBits(PATTERN,12,32*7); // 0-7; unaligned start/end

    assert(bits->Read(  0,32) == 0x246dbeef);
    assert(bits->Read( 32,32) == 0xdeadbeef);
    assert(bits->Read( 64,32) == 0xdeadbeef);
    assert(bits->Read( 96,32) == 0xdeadbeef);
    assert(bits->Read(128,32) == 0xdeadbeef);
    assert(bits->Read(160,32) == 0xdeadbeef);
    assert(bits->Read(192,32) == 0xdeadbeef);
    assert(bits->Read(224,32) == 0xdeabca09);

    bits = setup();
    bits->StoreBits(PATTERN,96,12); // 3-3; single front-aligned

    assertUnchanged(0,2);
    assert(bits->Read( 96,32) == 0xdeacdef0);
    assertUnchanged(4,7);

    bits = setup();
    bits->StoreBits(PATTERN,96+20,12); // 3-3; single back-aligned

    assertUnchanged(0,2);
    assert(bits->Read( 96,32) == 0x9abcdeef);
    assertUnchanged(4,7);

    bits = setup();
    bits->StoreBits(PATTERN,96+8,16); // 3-3; single unaligned

    assertUnchanged(0,2);
    assert(bits->Read( 96,32) == 0x9aadbef0);
    assertUnchanged(4,7);
  }

  void BitVector_Test::Test_bitVectorLong()
  {
    BitVector<256>* bits = setup();

    assert(bits->ReadLong(16, 64) == HexU64(0x13571112,0x13141234));
    assert(bits->ReadLong(0, 32) == HexU64(0x0,0x24681357));
    assert(bits->ReadLong(96, 32) == HexU64(0x0,0x9abcdef0));
    assert(bits->ReadLong(160, 36) == HexU64(0x8,0x76543214));

    bits->WriteLong(16,64,HexU64(0x13000012,0x13141234));
    assert(bits->ReadLong(16, 64) == HexU64(0x13000012,0x13141234));

    bits->WriteLong(24,16,0x5711L);
    assert(bits->ReadLong(16, 64) == HexU64(0x13571112,0x13141234));

    bits->WriteLong(192,64,0x0L);
    assert(bits->ReadLong(160, 64) == HexU64(0x87654321,0x00000000));

    assert(bits->ReadLong(192, 64) == 0x0L);
    bits->WriteLong(192,64,(u64) -1L);
    assert(bits->ReadLong(192, 64) == (u64) -1L);
  }

} /* namespace MFM */
