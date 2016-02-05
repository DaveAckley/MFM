#include "assert.h"
#include "UlamRef_Test.h"
#include "itype.h"

namespace MFM {

  typedef UlamRef<TestEventConfig> TestUlamRef;

  const u32 vals[3] =
    {
      0x87654321, // 0..31
      0x44332211, // 32..63
      0xfedbca09  // 64..95
    };

  TestAtom UlamRef_Test::setup()
  {

    TestAtom t;
    BitVector<96> & bv = t.GetBits();
    for (u32 i = 0; i < 96/32; ++i) {
      bv.Write(i*32, 32, vals[i]);
    }

    return t;
  }

  void UlamRef_Test::Test_RunTests() {
    Test_UlamRefCtors();
    Test_UlamRefRead();
    Test_UlamRefReadLong();
    Test_UlamRefWrite();
    Test_UlamRefWriteLong();
  }

  void UlamRef_Test::Test_UlamRefRead()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestUlamRef ur(i*32, 32, t, 0);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.Read() == vals[i]);
      }
    }

    {
      TestAtom t = setup();
      {
        TestUlamRef ur(16, 24, t, 0);
        assert(ur.Read() == 0x432144);
      }
      {
        TestUlamRef ur(4, 32, t, 0);
        assert(ur.Read() == 0x76543214);
      }
      {
        TestUlamRef ur(32+16, 32, t, 0);
        assert(ur.Read() == 0x2211fedb);
      }
      {
        TestUlamRef ur(28, 4, t, 0);
        assert(ur.Read() == 0x1);
      }
      {
        TestUlamRef ur(32*2 - 8, 8, t, 0);
        assert(ur.Read() == 0x11);
      }
    }
  }

  void UlamRef_Test::Test_UlamRefReadLong()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestUlamRef ur(i*32, 32, t, 0);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.ReadLong() == vals[i]);
      }
    }

    {
      TestAtom t = setup();
      {
        TestUlamRef ur(0, 64, t, 0);
        assert(ur.ReadLong() == HexU64(0x87654321,0x44332211));
      }
      {
        TestUlamRef ur(4, 64, t, 0);
        assert(ur.ReadLong() == HexU64(0x76543214,0x4332211F));
      }
      {
        TestUlamRef ur(28, 40, t, 0);
        assert(ur.ReadLong() == HexU64(0x14,0x4332211F));
      }
      {
        TestUlamRef ur(0, 36, t, 0);
        assert(ur.ReadLong() == HexU64(0x8,0x76543214));
      }
      {
        TestUlamRef ur(32, 36, t, 0);
        assert(ur.ReadLong() == HexU64(0x4,0x4332211F));
      }
      {
        TestUlamRef ur(16, 64, t, 0);
        assert(ur.ReadLong() == HexU64(0x43214433,0x2211fedb));
      }
      {
        TestUlamRef ur(31, 2, t, 0);
        assert(ur.ReadLong() == 0x2);
      }
      {
        TestUlamRef ur(32, 64, t, 0);
        assert(ur.ReadLong() == HexU64(0x44332211,0xfedbca09));
      }
      {
        TestUlamRef ur(32, 60, t, 0);
        assert(ur.ReadLong() == HexU64(0x4433221,0x1fedbca0));
      }
    }
  }

  void UlamRef_Test::Test_UlamRefWrite()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestUlamRef ur(i*32, 32, t, 0);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.Read() == vals[i]);
        ur.Write(i);
        assert(ur.Read() == i);
      }
    }
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/8; ++i) {
        TestUlamRef ur(i*8, 8, t, 0);

        assert(ur.GetPos() == i*8);
        assert(ur.GetLen() == 8);

        ur.Write(27*i); // will start overflowing
      }
      for (u32 i = 0; i < 96/8; ++i) {
        TestUlamRef ur(i*8, 8, t, 0);

        assert(ur.GetPos() == i*8);
        assert(ur.GetLen() == 8);

        assert(ur.Read() == ((27*i)&0xff));
      }
    }

    {
      TestAtom t = setup();
      {
        TestUlamRef ur(16, 24, t, 0);
        assert(ur.Read() == 0x432144);

        TestUlamRef ur2(24, 8, t, 0);
        ur2.Write(0xcd);

        assert(ur.Read() == 0x43cd44);

        TestUlamRef ur2b(8, 32, t, 0);
        ur2b.Write(0xfeedface);

        assert(ur.Read() == 0xedface);

        TestUlamRef ur3(0, 32, t, 0);
        assert(ur3.Read() == 0x87feedfa);

        TestUlamRef ur4(32, 32, t, 0);
        assert(ur4.Read() == 0xce332211);

        TestUlamRef ur5(64, 32, t, 0);
        assert(ur5.Read() == 0xfedbca09);
      }
    }
  }

  void UlamRef_Test::Test_UlamRefWriteLong()
  {
    {
      TestAtom t = setup();
      for (u32 i = 0; i < 96/32; ++i) {
        TestUlamRef ur(i*32, 32, t, 0);

        assert(ur.GetPos() == i*32);
        assert(ur.GetLen() == 32);

        assert(ur.Read() == vals[i]);
        ur.WriteLong(i);
        assert(ur.Read() == i);
      }
    }
    {
      TestAtom t = setup();
      TestUlamRef ur(28, 60, t, 0);

      ur.WriteLong(HexU64(0xdeadbee,0xfd00df00));
      assert(ur.ReadLong() == HexU64(0xdeadbee,0xfd00df00));

      TestUlamRef ur0(0, 32, t, 0);
      TestUlamRef ur1(32, 32, t, 0);
      TestUlamRef ur2(64, 32, t, 0);

      assert(ur0.Read() == 0x8765432d);
      assert(ur0.ReadLong() == HexU64(0x0,0x8765432d));

      assert(ur1.Read() == 0xeadbeefd);
      assert(ur1.ReadLong() == HexU64(0x0,0xeadbeefd));

      assert(ur2.Read() == 0x00df0009);
      assert(ur2.ReadLong() == HexU64(0x0,0x00df0009));
    }

    {
      TestAtom t = setup();

      TestUlamRef ur(31, 34, t, 0);

      ur.WriteLong(-1L);

      TestUlamRef ur0(0, 32, t, 0);
      TestUlamRef ur1(32, 32, t, 0);
      TestUlamRef ur2(64, 32, t, 0);

      assert(ur0.Read() == 0x87654321);
      assert(ur1.Read() == 0xffffffff);
      assert(ur2.Read() == 0xfedbca09);

      ur.WriteLong(0L);

      assert(ur0.Read() == 0x87654320);
      assert(ur1.Read() == 0x0);
      assert(ur2.Read() == 0x7edbca09);
    }
    {
      TestAtom t = setup();

      TestUlamRef ur01(0, 64, t, 0);
      TestUlamRef ur12(32, 64, t, 0);

      TestUlamRef ur0(0, 32, t, 0);
      TestUlamRef ur1(32, 32, t, 0);
      TestUlamRef ur2(64, 32, t, 0);

      assert(ur0.Read() == 0x87654321);
      assert(ur1.Read() == 0x44332211);
      assert(ur2.Read() == 0xfedbca09);

      ur01.WriteLong(HexU64(0x01234567,0x89abcdef));

      assert(ur0.Read() == 0x01234567);
      assert(ur1.Read() == 0x89abcdef);
      assert(ur2.Read() == 0xfedbca09);

      ur12.WriteLong(HexU64(0x246800ee,0xee00aaaa));
      assert(ur0.Read() == 0x01234567);
      assert(ur1.Read() == 0x246800ee);
      assert(ur2.Read() == 0xee00aaaa);

    }
  }

  void UlamRef_Test::Test_UlamRefCtors()
  {
    {
      TestAtom t = setup();
      TestUlamRef ur(0, 10, t, 0);
      assert(ur.GetPos() == 0);
      assert(ur.GetLen() == 10);

      TestUlamRef ur2(ur, 4, 6, 0);
      assert(ur2.GetPos() == 4);
      assert(ur2.GetLen() == 6);

      TestUlamRef ur3(ur2, 2, 1, 0);
      assert(ur3.GetPos() == 6);
      assert(ur3.GetLen() == 1);
    }

    {
      TestAtom t = setup();
      TestUlamRef ur(0, 96, t, 0);
      assert(ur.GetPos() == 0);
      assert(ur.GetLen() == 96);

      TestUlamRef ur2(ur, 80, 16, 0);
      assert(ur2.GetPos() == 80);
      assert(ur2.GetLen() == 16);

      TestUlamRef ur3(ur2, 10, 6, 0);
      assert(ur3.GetPos() == 90);
      assert(ur3.GetLen() == 6);
    }
  }

} /* namespace MFM */
