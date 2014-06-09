#include "assert.h"
#include "ByteSource_Test.h"

#include "ZStringByteSource.h"
#include "CharBufferByteSink.h"

namespace MFM {

  static ZStringByteSource tester("");

  static void Test_Basic() {
    int ch;
    tester.Reset();

    ch = tester.Read();
    assert(ch == -1);

    ch = tester.Read();  // Still EOF
    assert(ch == -1);

    tester.Reset("hi");
    assert(tester.Read() == 'h');
    assert(tester.Read() == 'i');
    assert(tester.Read() == -1);
    assert(tester.Read() == -1);
  }

  static void Test_Unread() {
    tester.Reset("abc");
    assert(tester.Read() == 'a');

    assert(tester.Read() == 'b');
    tester.Unread();
    assert(tester.Read() == 'b');
    tester.Unread();
    assert(tester.Read() == 'b');

    assert(tester.Read() == 'c');
    tester.Unread();
    assert(tester.Read() == 'c');
    tester.Unread();
    assert(tester.Read() == 'c');

    assert(tester.Read() == -1);
    tester.Unread();
    assert(tester.Read() == -1);
    tester.Unread();
    assert(tester.Read() == -1);
  }

  static void Test_Scan() {
    s32 val;

    tester.Reset("123");
    assert(tester.Scan(val, Format::DEC));
    assert(val == 123);

    tester.Reset();
    assert(tester.Scan(val, Format::OCT));
    assert(val == 0123);

    tester.Reset();
    assert(tester.Scan(val, Format::HEX));
    assert(val == 0x123);

    tester.Reset();
    assert(tester.Scan(val, (Format::Type) 5));  // Base 5
    assert(val == 1*(5*5) + 2*(5) + 3);

    tester.Reset();
    assert(tester.Scan(val, (Format::Type) 36));  // Base 36 0..9a..z
    assert(val == 1*(36*36) + 2*(36) + 3);

    tester.Reset();
    assert(tester.Scan(val, Format::BIN));
    assert(val == 1);

    assert(!tester.Scan(val, Format::BIN));  // Looking at the 2
    assert(tester.Scan(val, Format::DEC));
    assert(val == 23);

    u32 uval;

    tester.Reset("deaDBEefs");
    assert(tester.Scan(uval, Format::HEX));
    assert(uval == 0xdeadbeef);
    assert(tester.Scan(uval, Format::BYTE));
    assert(uval == 's');
    assert(!tester.Scan(uval, Format::BYTE));
  }

  static void Test_ScanSetFormat() {

    tester.Reset("");
    const char * p;

    p = "[a]";
    tester.ScanSetFormat(DevNull, p);
    assert(*p == 0);

    p = "[^a-zA-Z0-9-]!";
    tester.ScanSetFormat(DevNull, p);
    assert(*p == '!');

    p = "[]^-]@";
    tester.ScanSetFormat(DevNull, p);
    assert(*p == '@');
  }

  static CharBufferByteSink<100> obuf;

  static void Test_ScanSet() {

    tester.Reset("foodz");
    obuf.Reset();
    assert(4 == tester.ScanSet(obuf, "[d-o]"));
    assert(obuf.Equals("food"));
    assert(tester.Read() == 'z');

    ///

    tester.Reset("\n\t Foo(Bar, Gah)");
    obuf.Reset();
    assert(tester.SkipWhitespace() >= 0);
    assert(3 == tester.ScanSet(obuf, "[a-zA-Z]"));
    assert(obuf.Equals("Foo"));

    assert(tester.SkipWhitespace() >= 0);
    assert(1 == tester.SkipSet("[(]"));
    assert(tester.SkipWhitespace() >= 0);

    obuf.Reset();
    assert(3 == tester.ScanSet(obuf, "[a-zA-Z]"));
    assert(obuf.Equals("Bar"));

    assert(tester.SkipWhitespace() >= 0);
    assert(1 == tester.SkipSet("[,]"));
    assert(tester.SkipWhitespace() >= 0);

    obuf.Reset();
    assert(3 == tester.ScanSet(obuf, "[a-zA-Z]"));
    assert(obuf.Equals("Gah"));

    assert(tester.SkipWhitespace() >= 0);
    assert(1 == tester.SkipSet("[)]"));
    assert(tester.SkipWhitespace() < 0);  // EOF immediately

    ///

    tester.Reset("foodz");
    obuf.Reset();
    assert(1 == tester.ScanSet(obuf, "[^o]"));
    assert(obuf.Equals("f"));
    assert(2 == tester.ScanSet(obuf, "[o]"));
    assert(obuf.Equals("foo"));                  // Not 'oo' -- we didn't obuf.Reset()
    assert(2 == tester.ScanSet(obuf, "[^o]"));
    assert(obuf.Equals("foodz"));
    assert(-1 == tester.ScanSet(obuf, "[^o]"));  // Nothing left
    assert(obuf.Equals("foodz"));

    ///

    tester.Reset("When \n\n  in \n the \t course \n of \t foo.");
    obuf.Reset();
    while (tester.SkipWhitespace() >= 0) {
      tester.ScanSet(obuf,ByteSource::NON_WHITESPACE_SET);
    }
    assert(obuf.Equals("Wheninthecourseoffoo."));

  }

  void ByteSource_Test::Test_RunTests() {
    Test_Basic();
    Test_Unread();
    Test_Scan();
    Test_ScanSetFormat();
    Test_ScanSet();
  }

} /* namespace MFM */


