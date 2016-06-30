#include "assert.h"
#include "UlamRef_Test.h"
#include "itype.h"

namespace MFM {

  typedef UlamRef<TestEventConfig> TestUlamRef;
  typedef ElementTable<TestEventConfig> TestElementTable;
  typedef UlamContext<TestEventConfig> TestUlamContext;

  const u32 vals[3] =
    {
      0x87654321 >> 25, // 0..31
      (0x87654321 << 7) | (0x44332211 >> 25), // 32..63
      (0x44332211 << 7) | (0xfedbca09 >> 25)  // 64..95
    };

  const u32 svals[3] =
    {
      0x87654321, // 25..56
      0x44332211, // 57..88
      0xfedbca09  // 89..95
    };

  TestAtom UlamRef_Test::setup()
  {

    TestAtom t;
    BitVector<96> & bv = t.GetBits();
    for (u32 i = 0; i < 96/32; ++i) {
      bv.Write(i*32, 32, svals[i]);
    }

    return t;
  }

  void UlamRef_Test::Test_RunTests() {
    Test_UlamRefReadBV();
    Test_UlamRefWriteBV();
    Test_UlamRefCtors();
    Test_UlamRefRead();
    Test_UlamRefReadLong();
    Test_UlamRefWrite();
    Test_UlamRefWriteLong();
    Test_UlamRefEffSelf();
  }

  void UlamRef_Test::Test_UlamRefRead()
  {
    TestElementTable tet;
    TestUlamContext tuc(tet);
    {
      AtomBitStorage<TestEventConfig> t(setup());
      for (u32 i = 0; i < 96/32; ++i) {
        u32 len = 32;
        TestUlamRef ur(i*32, len, t, 0, TestUlamRef::PRIMITIVE, tuc);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == len);

        assert(ur.Read() == svals[i]);
      }
    }

    {
      AtomBitStorage<TestEventConfig> t(setup());
      {
        TestUlamRef ur(16, 24, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.Read() == 0x432144);
      }
      {
        TestUlamRef ur(4, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.Read() == 0x76543214);
      }
      {
        TestUlamRef ur(32+16, 20, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.Read() == 0x2211f);
      }
      {
        TestUlamRef ur(28, 4, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.Read() == 0x1);
      }
      {
        TestUlamRef ur(32*2 - 8, 8, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.Read() == 0x11);
      }
    }
  }

  void UlamRef_Test::Test_UlamRefReadLong()
  {
    TestElementTable tet;
    TestUlamContext tuc(tet);
    {
      AtomBitStorage<TestEventConfig> t(setup());
      for (u32 i = 0; i < 96/32; ++i) {
        u32 len = 32;
        TestUlamRef ur(i*32, len, t, 0, TestUlamRef::PRIMITIVE, tuc);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == len);

        assert(ur.ReadLong() == svals[i]);
      }
    }

    {
      AtomBitStorage<TestEventConfig> t(setup());
      {
        TestUlamRef ur(0, 64, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x87654321,0x44332211));
      }
      {
        TestUlamRef ur(4, 64, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x76543214,0x4332211F));
      }
      {
        TestUlamRef ur(28, 40, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x14,0x4332211F));
      }
      {
        TestUlamRef ur(0, 36, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x8,0x76543214));
      }
      {
        TestUlamRef ur(32, 36, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x4,0x4332211F));
      }
      {
        TestUlamRef ur(16, 52, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x43214,0x4332211f));
      }
      {
        TestUlamRef ur(31, 2, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == 0x2);
      }
      {
        TestUlamRef ur(32, 36, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.ReadLong() == HexU64(0x4,0x4332211f));
      }
    }
  }

  void UlamRef_Test::Test_UlamRefWrite()
  {
    TestElementTable tet;
    TestUlamContext tuc(tet);

    {
      AtomBitStorage<TestEventConfig> t(setup());

      for (u32 i = 0; i < 96/32; ++i) {
        u32 len = 32;
        TestUlamRef ur(i*32, len, t, 0, TestUlamRef::PRIMITIVE, tuc);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == len);

        assert(ur.Read() == svals[i]);
        ur.Write(i);
        assert(ur.Read() == i);
      }
    }
    {
      AtomBitStorage<TestEventConfig> t(setup());

      for (u32 i = 0; i < 71/8; ++i) {
        TestUlamRef ur(i*8, 8, t, 0, TestUlamRef::PRIMITIVE, tuc);

        assert(ur.GetPos() == i*8);
        assert(ur.GetLen() == 8);

        ur.Write(27*i); // will start overflowing
      }
      for (u32 i = 0; i < 71/8; ++i) {
        TestUlamRef ur(i*8, 8, t, 0, TestUlamRef::PRIMITIVE, tuc);

        assert(ur.GetPos() == i*8);
        assert(ur.GetLen() == 8);

        assert(ur.Read() == ((27*i)&0xff));
      }
    }

    {
      AtomBitStorage<TestEventConfig> t(setup());
      {
        TestUlamRef ur(16, 24, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur.Read() == 0x432144);

        TestUlamRef ur2(24, 8, t, 0, TestUlamRef::PRIMITIVE, tuc);
        ur2.Write(0xcd);

        assert(ur.Read() == 0x43cd44);

        TestUlamRef ur2b(8, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
        ur2b.Write(0xfeedface);

        assert(ur.Read() == 0xedface);

        TestUlamRef ur3(0, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur3.Read() == 0x87feedfa);

        TestUlamRef ur4(32, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur4.Read() == 0xce332211);

        TestUlamRef ur5(64, 7, t, 0, TestUlamRef::PRIMITIVE, tuc);
        assert(ur5.Read() == 0xfe>>1);
      }
    }
  }

  void UlamRef_Test::Test_UlamRefWriteLong()
  {
    TestElementTable tet;
    TestUlamContext tuc(tet);

    {
      AtomBitStorage<TestEventConfig> t(setup());
      for (u32 i = 0; i < 96/32; ++i) {
        u32 len = 32;
        TestUlamRef ur(i*32, len, t, 0, TestUlamRef::PRIMITIVE, tuc);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == len);

        assert(ur.Read() == svals[i]);
        ur.WriteLong(i);
        assert(ur.Read() == i);
      }
    }
    {
      AtomBitStorage<TestEventConfig> t(setup());

      TestUlamRef ur(8, 60, t, 0, TestUlamRef::PRIMITIVE, tuc);

      ur.WriteLong(HexU64(0xdeadbee,0xfd00df00));
      assert(ur.ReadLong() == HexU64(0xdeadbee,0xfd00df00));

      TestUlamRef ur0(0, 8, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur1(8, 36, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur2(44, 60-36, t, 0, TestUlamRef::PRIMITIVE, tuc);

      assert(ur0.Read() == 0x87);
      assert(ur0.ReadLong() == HexU64(0x0,0x87));

      assert(ur1.ReadLong() == HexU64(0xd,0xeadbeefd));

      assert(ur2.Read() == 0x00df00);
      assert(ur2.ReadLong() == HexU64(0x0,0x00df00));
    }

    {
      AtomBitStorage<TestEventConfig> t(setup());

      TestUlamRef ur(31, 34, t, 0, TestUlamRef::PRIMITIVE, tuc);

      ur.WriteLong(-1L);

      TestUlamRef ur0(0, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur1(32, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur2(64, 4, t, 0, TestUlamRef::PRIMITIVE, tuc);

      assert(ur0.Read() == 0x87654321);
      assert(ur1.Read() == 0xffffffff);
      assert(ur2.Read() == 0xf);

      ur.WriteLong(0L);

      assert(ur0.Read() == 0x87654320);
      assert(ur1.Read() == 0x0);
      assert(ur2.Read() == 0x7);
    }
    {
      AtomBitStorage<TestEventConfig> t(setup());

      TestUlamRef ur01(0, 64, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur12(32, 68-32, t, 0, TestUlamRef::PRIMITIVE, tuc);

      TestUlamRef ur0(0, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur1(32, 32, t, 0, TestUlamRef::PRIMITIVE, tuc);
      TestUlamRef ur2(64, 71-64, t, 0, TestUlamRef::PRIMITIVE, tuc);

      assert(ur0.Read() == 0x87654321);
      assert(ur1.Read() == 0x44332211);
      assert(ur2.Read() == 0x7f);

      ur01.WriteLong(HexU64(0x01234567,0x89abcdef));

      assert(ur0.Read() == 0x01234567);
      assert(ur1.Read() == 0x89abcdef);
      assert(ur2.Read() == 0x7f);

      ur12.WriteLong(HexU64(0x246800ee,0xee00aaaa));
      assert(ur0.Read() == 0x01234567);
      assert(ur1.Read() == 0xeee00aaa);
      assert(ur2.Read() == 0x57);

    }
  }

  void UlamRef_Test::Test_UlamRefCtors()
  {
    TestElementTable tet;
    TestUlamContext tuc(tet);
    {
      AtomBitStorage<TestEventConfig> t(setup());

      TestUlamRef ur(0, 10, t, 0, TestUlamRef::PRIMITIVE, tuc);
      assert(ur.GetPos() == 0);
      assert(ur.GetLen() == 10);

      TestUlamRef ur2(ur, 4, 6, 0, TestUlamRef::PRIMITIVE);
      assert(ur2.GetPos() == 4);
      assert(ur2.GetLen() == 6);

      TestUlamRef ur3(ur2, 2, 1, 0, TestUlamRef::PRIMITIVE);
      assert(ur3.GetPos() == 6);
      assert(ur3.GetLen() == 1);
    }

    {
      AtomBitStorage<TestEventConfig> t(setup());

      TestUlamRef ur(0, 71, t, 0, TestUlamRef::PRIMITIVE, tuc);
      assert(ur.GetPos() == 0);
      assert(ur.GetLen() == 71);

      TestUlamRef ur2(ur, 55, 16, 0, TestUlamRef::PRIMITIVE);
      assert(ur2.GetPos() == 55);
      assert(ur2.GetLen() == 16);

      TestUlamRef ur3(ur2, 10, 6, 0, TestUlamRef::PRIMITIVE);
      assert(ur3.GetPos() == 65);
      assert(ur3.GetLen() == 6);
    }
  }

  void UlamRef_Test::Test_UlamRefEffSelf()
  {
    TestElementTable tet;
    TestUlamContext tuc(tet);
    {
      AtomBitStorage<TestEventConfig> t(setup());
      TestUlamRef ur(0, 71, t, 0, TestUlamRef::PRIMITIVE, tuc);
      assert(ur.GetEffectiveSelf() == 0);
    }
  }


  void UlamRef_Test::Test_UlamRefReadBV()
  {
    {
      BitVectorBitStorage<TestEventConfig,BitVector<320> > t;
      for (u32 i = 0; i < 320/8; ++i) // init to 0x010203..2728
        t.Write(i * 8, 8, i + 1);

      BitVector<4> b04;
      BitVector<8> b08;
      BitVector<16> b16;
      BitVector<32> b32;
      BitVector<64> b64;
      BitVector<80> b80;

      t.ReadBV<4>(0, b04);

      assert(b04.Read(0,4) == 0x0);
      t.ReadBV(4, b04);
      assert(b04.Read(0,4) == 0x1);
      t.ReadBV(28, b04);
      assert(b04.Read(0,4) == 0x4);

      t.ReadBV(0, b16);
      assert(b16.Read(0,16) == 0x0102);
      t.ReadBV(8, b16);
      assert(b16.Read(0,16) == 0x0203);
      t.ReadBV(24, b16);
      assert(b16.Read(0,16) == 0x0405);

      t.ReadBV(0, b32);
      assert(b32.Read(0,32) == 0x01020304);
      t.ReadBV(8, b32);
      assert(b32.Read(0,32) == 0x02030405);

      t.ReadBV(0, b64);
      assert(b64.ReadLong(0,64) == 0x0102030405060708L);
      t.ReadBV(8, b64);
      assert(b64.ReadLong(0,64) == 0x0203040506070809L);

      t.ReadBV(0, b80);
      assert(b80.ReadLong(0,64) ==  0x0102030405060708L);
      assert(b80.Read(64,16) ==     0x090a);
      t.ReadBV(24, b80);
      assert(b80.ReadLong(0,64) ==  0x0405060708090a0bL);
      assert(b80.Read(64,16) ==     0x0c0d);

    }

    {
      BitVectorBitStorage<TestEventConfig,BitVector<16> > t;
      t.Write(0,16,0xab98);

      BitVector<4> b04;
      BitVector<8> b08;
      BitVector<16> b16;

      t.ReadBV(0, b04);
      assert(b04.Read(0,4) == 0xa);
      t.ReadBV(4, b04);
      assert(b04.Read(0,4) == 0xb);
      t.ReadBV(12, b04);
      assert(b04.Read(0,4) == 0x8);

      t.ReadBV(0, b08);
      assert(b08.Read(0,8) == 0xab);
      t.ReadBV(4, b08);
      assert(b08.Read(0,8) == 0xb9);
      t.ReadBV(8, b08);
      assert(b08.Read(0,8) == 0x98);

      t.ReadBV(0, b16);
      assert(b16.Read(0,16) == 0xab98);

    }

  }


  void UlamRef_Test::Test_UlamRefWriteBV()
  {

    {
      BitVectorBitStorage<TestEventConfig,BitVector<320> > t;
      for (u32 i = 0; i < 320/8; ++i) // init to 0x010203..2728
        t.Write(i * 8, 8, i + 1);

      BitVector<4> b04(0xe);
      BitVector<8> b08(0x3f);
      BitVector<16> b16(0xabcd);
      BitVector<32> b32(0x12345678);

      t.WriteBV(0, b04);
      assert(t.Read(0,32) == 0xe1020304);
      t.WriteBV(4, b04);
      assert(t.Read(0,32) == 0xee020304);
      t.WriteBV(28, b04);
      assert(t.Read(0,32) == 0xee02030e);

      t.WriteBV(0, b08);
      assert(t.Read(0,32) == 0x3f02030e);
      t.WriteBV(8, b08);
      assert(t.Read(0,32) == 0x3f3f030e);
      t.WriteBV(24, b08);
      assert(t.Read(0,32) == 0x3f3f033f);

      t.WriteBV(0, b16);
      assert(t.Read(0,32) == 0xabcd033f);
      t.WriteBV(8, b16);
      assert(t.Read(0,32) == 0xababcd3f);
      t.WriteBV(24, b16);
      assert(t.ReadLong(0,64) == 0xababcdabcd060708L);

      t.WriteBV(0, b32);
      assert(t.ReadLong(0,64) == 0x12345678cd060708L);
      t.WriteBV(8, b32);
      assert(t.ReadLong(0,64) == 0x1212345678060708L);

    }

    {
      BitVectorBitStorage<TestEventConfig,BitVector<16> > t;
      t.Write(0,16,0xab98);

      BitVector<4> b04(0x9);
      BitVector<8> b08(0xba);
      BitVector<16> b16(0xdead);

      assert(t.Read(0,16) == 0xab98);
      t.WriteBV(0, b04);
      assert(t.Read(0,16) == 0x9b98);
      t.WriteBV(4, b04);
      assert(t.Read(0,16) == 0x9998);
      t.WriteBV(12, b04);
      assert(t.Read(0,16) == 0x9999);

      t.WriteBV(0, b08);
      assert(t.Read(0,16) == 0xba99);
      t.WriteBV(4, b08);
      assert(t.Read(0,16) == 0xbba9);
      t.WriteBV(8, b08);
      assert(t.Read(0,16) == 0xbbba);
    }

  }



} /* namespace MFM */
