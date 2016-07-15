#include "assert.h"
#include "BitRef_Test.h"
#include "itype.h"

namespace MFM {

  typedef BitRef<TestEventConfig> TestBitRef;

  const u32 vals[3] =
    {
      0x87654321, // 0..31
      0x44332211, // 32..63
      0xfedbca09  // 64..95
    };

  TestAtom BitRef_Test::setup()
  {

    TestAtom t;
    BitVector<96> & bv = t.GetBits();
    for (u32 i = 0; i < 96/32; ++i) {
      bv.Write(i*32, 32, vals[i]);
    }

    return t;
  }

  void BitRef_Test::Test_RunTests() {
    Test_BitRefCtors();
    Test_BitRefRead();
    Test_BitRefReadLong();
    Test_BitRefWrite();
    Test_BitRefWriteLong();
  }

  void BitRef_Test::Test_BitRefRead()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestBitRef ur(i*32, 32);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.Read(t) == vals[i]);
      }
    }

    {
      TestAtom t = setup();
      {
        TestBitRef ur(16, 24);
        assert(ur.Read(t) == 0x432144);
      }
      {
        TestBitRef ur(4, 32);
        assert(ur.Read(t) == 0x76543214);
      }
      {
        TestBitRef ur(32+16, 32);
        assert(ur.Read(t) == 0x2211fedb);
      }
      {
        TestBitRef ur(28, 4);
        assert(ur.Read(t) == 0x1);
      }
      {
        TestBitRef ur(32*2 - 8, 8);
        assert(ur.Read(t) == 0x11);
      }
    }
  }

  void BitRef_Test::Test_BitRefReadLong()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestBitRef ur(i*32, 32);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.ReadLong(t) == vals[i]);
      }
    }

    {
      TestAtom t = setup();
      {
        TestBitRef ur(0, 64);
        assert(ur.ReadLong(t) == HexU64(0x87654321,0x44332211));
      }
      {
        TestBitRef ur(4, 64);
        assert(ur.ReadLong(t) == HexU64(0x76543214,0x4332211F));
      }
      {
        TestBitRef ur(28, 40);
        assert(ur.ReadLong(t) == HexU64(0x14,0x4332211F));
      }
      {
        TestBitRef ur(0, 36);
        assert(ur.ReadLong(t) == HexU64(0x8,0x76543214));
      }
      {
        TestBitRef ur(32, 36);
        assert(ur.ReadLong(t) == HexU64(0x4,0x4332211F));
      }
      {
        TestBitRef ur(16, 64);
        assert(ur.ReadLong(t) == HexU64(0x43214433,0x2211fedb));
      }
      {
        TestBitRef ur(31, 2);
        assert(ur.ReadLong(t) == 0x2);
      }
      {
        TestBitRef ur(32, 64);
        assert(ur.ReadLong(t) == HexU64(0x44332211,0xfedbca09));
      }
      {
        TestBitRef ur(32, 60);
        assert(ur.ReadLong(t) == HexU64(0x4433221,0x1fedbca0));
      }
    }
  }

  void BitRef_Test::Test_BitRefWrite()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestBitRef ur(i*32, 32);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.Read(t) == vals[i]);
        ur.Write(t,i);
        assert(ur.Read(t) == i);
      }
    }
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/8; ++i) {
        TestBitRef ur(i*8, 8);

        assert(ur.GetPos() == i*8);
        assert(ur.GetLen() == 8);

        ur.Write(t,27*i); // will start overflowing
      }
      for (u32 i = 0; i < 96/8; ++i) {
        TestBitRef ur(i*8, 8);

        assert(ur.GetPos() == i*8);
        assert(ur.GetLen() == 8);

        assert(ur.Read(t) == ((27*i)&0xff));
      }
    }

    {
      TestAtom t = setup();
      {
        TestBitRef ur(16, 24);
        assert(ur.Read(t) == 0x432144);

        TestBitRef ur2(24, 8);
        ur2.Write(t,0xcd);

        assert(ur.Read(t) == 0x43cd44);

        TestBitRef ur2b(8, 32);
        ur2b.Write(t,0xfeedface);

        assert(ur.Read(t) == 0xedface);

        TestBitRef ur3(0, 32);
        assert(ur3.Read(t) == 0x87feedfa);

        TestBitRef ur4(32, 32);
        assert(ur4.Read(t) == 0xce332211);

        TestBitRef ur5(64, 32);
        assert(ur5.Read(t) == 0xfedbca09);
      }
    }
  }

  void BitRef_Test::Test_BitRefWriteLong()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestBitRef ur(i*32, 32);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.Read(t) == vals[i]);
        ur.WriteLong(t,i);
        assert(ur.Read(t) == i);
      }
    }
    {
      TestAtom t = setup();
      TestBitRef ur(28, 60);

      ur.WriteLong(t,HexU64(0xdeadbee,0xfd00df00));
      assert(ur.ReadLong(t) == HexU64(0xdeadbee,0xfd00df00));

      TestBitRef ur0(0, 32);
      TestBitRef ur1(32, 32);
      TestBitRef ur2(64, 32);

      assert(ur0.Read(t) == 0x8765432d);
      assert(ur0.ReadLong(t) == HexU64(0x0,0x8765432d));

      assert(ur1.Read(t) == 0xeadbeefd);
      assert(ur1.ReadLong(t) == HexU64(0x0,0xeadbeefd));

      assert(ur2.Read(t) == 0x00df0009);
      assert(ur2.ReadLong(t) == HexU64(0x0,0x00df0009));
    }

    {
      TestAtom t = setup();

      TestBitRef ur(31, 34);

      ur.WriteLong(t,-1L);

      TestBitRef ur0(0, 32);
      TestBitRef ur1(32, 32);
      TestBitRef ur2(64, 32);

      assert(ur0.Read(t) == 0x87654321);
      assert(ur1.Read(t) == 0xffffffff);
      assert(ur2.Read(t) == 0xfedbca09);

      ur.WriteLong(t,0L);

      assert(ur0.Read(t) == 0x87654320);
      assert(ur1.Read(t) == 0x0);
      assert(ur2.Read(t) == 0x7edbca09);
    }
    {
      TestAtom t = setup();

      TestBitRef ur01(0, 64);
      TestBitRef ur12(32, 64);

      TestBitRef ur0(0, 32);
      TestBitRef ur1(32, 32);
      TestBitRef ur2(64, 32);

      assert(ur0.Read(t) == 0x87654321);
      assert(ur1.Read(t) == 0x44332211);
      assert(ur2.Read(t) == 0xfedbca09);

      ur01.WriteLong(t,HexU64(0x01234567,0x89abcdef));

      assert(ur0.Read(t) == 0x01234567);
      assert(ur1.Read(t) == 0x89abcdef);
      assert(ur2.Read(t) == 0xfedbca09);

      ur12.WriteLong(t,HexU64(0x246800ee,0xee00aaaa));
      assert(ur0.Read(t) == 0x01234567);
      assert(ur1.Read(t) == 0x246800ee);
      assert(ur2.Read(t) == 0xee00aaaa);

    }
  }

  void BitRef_Test::Test_BitRefCtors()
  {
    {
      TestBitRef ur(0, 10);
      assert(ur.GetPos() == 0);
      assert(ur.GetLen() == 10);
    }

    {
      TestBitRef ur(0, 96);
      assert(ur.GetPos() == 0);
      assert(ur.GetLen() == 96);
    }
  }

} /* namespace MFM */
