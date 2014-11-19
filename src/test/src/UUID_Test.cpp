#include "assert.h"
#include "UUID_Test.h"
#include "CharBufferByteSink.h"
#include "ZStringByteSource.h"
#include <string.h>        /* For memcpy */

namespace MFM {
  typedef CharBufferByteSink<1024> CBS1K;

  static CBS1K tbuf;
  static ZStringByteSource fbuf("");

  static void Test_Basic() {
    UUID u1("Hi",1,2,3,4);

    assert(!strcmp(u1.GetLabel(),"Hi"));
    assert(u1.GetUUIDVersion() == UUID::API_VERSION);
    assert(u1.GetElementVersion() == 1);
    assert(u1.GetHexDate() == 2);
    assert(u1.GetHexTime() == 3);
    assert(u1.GetConfigurationCode() == 4);

    UUID u2("Hi", 1, 0x20140516, 0x190447, 4);
    assert(!strcmp(u2.GetLabel(),"Hi"));
    assert(u2.GetElementVersion() == 1);
    assert(u2.GetHexDate() == 0x20140516);
    assert(u2.GetHexTime() == 0x190447);
    assert(u2.GetConfigurationCode() == 4);

    assert(u1.CompatibleLabel(u2));
    assert(u2.CompatibleLabel(u1));
    assert(u1.CompatibleLabel(u1));
    assert(u2.CompatibleLabel(u2));

    UUID u2a("Ho", 1, 0x20140516, 0x190447, 4);
    assert(!u1.CompatibleLabel(u2a));
    assert(!u2.CompatibleLabel(u2a));
    assert(u2a.CompatibleLabel(u2a));

    assert(u1.CompatibleAPIVersion(u2));
    assert(u2.CompatibleAPIVersion(u1));
    assert(u1.CompatibleAPIVersion(u1));
    assert(u2.CompatibleAPIVersion(u2));

    assert(!u1.CompatibleButStrictlyNewer(u2));
    assert(!u1.CompatibleButStrictlyNewer(u1));
    assert(u2.CompatibleButStrictlyNewer(u1));
    assert(!u2.CompatibleButStrictlyNewer(u2));

    UUID u3("Hi",2,0x20140516,0x191339, 0);
    assert(u1.CompatibleLabel(u3));
    assert(u2.CompatibleLabel(u3));
    assert(u3.CompatibleLabel(u3));

    assert(!u3.CompatibleAPIVersion(u1));
    assert(!u3.CompatibleAPIVersion(u2));
    assert(u3.CompatibleAPIVersion(u3));

    assert(!u3.CompatibleButStrictlyNewer(u1));
    assert(!u3.CompatibleButStrictlyNewer(u2));
    assert(!u3.CompatibleButStrictlyNewer(u3));

  }

  static void Test_Vprintf_Result(const char * result, const char * format, ... ) {
    va_list ap;
    tbuf.Reset();
    va_start(ap, format);
    tbuf.Vprintf(format, ap);
    va_end(ap);
    assert(tbuf.Equals(result));
  }

  static void Test_Print() {
    UUID u1("Sorter",1,2,3,4);
    Test_Vprintf_Result("foo[Sorter-1111141213]bar", "foo[%@]bar", &u1);
    Test_Vprintf_Result("([Sorter-1111141213])", "([%#@])", 1234, &u1);

    UUID u2("Hi", 2, 0x20140516, 0x191339, 0x8899);
    Test_Vprintf_Result("foo/Hi-1112488998201405166191339/bar", "foo/%@/bar", &u2);

    UUID u3("VacuousSnareDrumwithKickbackReburn",
            87, 0x20140516, 0x214811, 0x109);
    Test_Vprintf_Result("[VacuousSnareDrumwithKickbackReburn-1128731098201405166214811]",
                        "[%@]", &u3);

    UUID u4("W", 1, 2, 3, 4);
    Test_Vprintf_Result("fooW-1111141213bar", "foo%@bar", &u4);
  }

  static void Test_Vprintf_Read(const UUID & u) {
    tbuf.Reset();
    tbuf.Printf("%@", &u);
    fbuf.Reset(tbuf.GetZString());
    UUID r(fbuf);
    assert(u == r);
    assert(fbuf.Read() == -1);
  }

  static void Test_Read() {
    Test_Vprintf_Read(UUID("Sorter",1, 2, 3, 4));
    Test_Vprintf_Read(UUID("Hi",2,0x20140516,0x191339, 0xaabb));
    Test_Vprintf_Read(UUID("VacuousSnareDrumwithKickbackReburn",
                           87, 0x20140516, 0x214811, 123));
    Test_Vprintf_Read(UUID("X", 1, 2, 3, 4));
  }

  void UUID_Test::Test_RunTests() {
    Test_Basic();
    Test_Print();
    Test_Read();
  }

} /* namespace MFM */
