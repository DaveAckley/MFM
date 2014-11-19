#include "assert.h"
#include "ByteSource_Test.h"

#include "ZStringByteSource.h"
#include "CharBufferByteSink.h"
#include "UUID.h"

namespace MFM {

  static ZStringByteSource tester("");

  static CharBufferByteSink<100> obuf;

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

  static void Test_ScanFieldwidths() {
    s32 val;

    tester.Reset("123");
    assert(!tester.Scan(val, Format::DEC, 0));
    assert(!tester.Scan(val, Format::DEC, 0));
    assert(tester.Scan(val, Format::DEC, 2));
    assert(val == 12);
    assert(tester.Scan(val, Format::DEC, 2));
    assert(val == 3);

    tester.Reset();
    assert(tester.Scan(val, Format::OCT, 1));
    assert(val == 1);
    assert(tester.Scan(val, Format::OCT, 2));
    assert(val == 023);

    tester.Reset();
    assert(tester.Scan(val, Format::HEX, 4));
    assert(val == 0x123);

    tester.Reset();
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 2);
    assert(!tester.Scan(val, Format::LEX32));

    tester.Reset("3246");
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 246);

    tester.Reset("40246");
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 246);

    tester.Reset("40246313421218");
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 246);
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 134);
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 12);
    assert(tester.Scan(val, Format::LEX32));
    assert(val == 8);
    assert(tester.Read() < 0);

    tester.Reset("010011000111"); // 0100 1100 0111
    assert(tester.Scan(val, Format::BIN));
    assert(val == 0x4c7);

    tester.Reset("010011000111"); // 0 1001 1000 111
    assert(tester.Scan(val, Format::BIN, 1));
    assert(val == 0);
    assert(tester.Scan(val, Format::BIN, 4));
    assert(val == 9);
    assert(tester.Scan(val, Format::BIN, 4));
    assert(val == 8);
    assert(tester.Scan(val, Format::BIN, 4));
    assert(val == 7);

    tester.Reset("abcdefghijklmnopqrstuvwxyz");
    obuf.Reset();
    assert(tester.Scan(obuf, 0));

    assert(tester.Scan(obuf, 1));
    assert(obuf.Equals("a"));

    assert(tester.Scan(obuf, 3));
    assert(obuf.Equals("abcd"));

    obuf.Reset();
    assert(tester.Scan(obuf, 19));
    assert(obuf.Equals("efghijklmnopqrstuvw"));

    obuf.Reset();
    assert(tester.Scan(obuf, 3));
    assert(obuf.Equals("xyz"));

    assert(tester.Scan(obuf, 0));
    assert(!tester.Scan(obuf, 1));

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

  static void Test_PrintfScanfRoundTrip(const char * format, u32 value) {
    obuf.Reset();
    obuf.Printf(format, value);
    tester.Reset(obuf.GetZString());
    u32 recovered = value + 1;
    assert(tester.Scanf(format, &recovered) > 0);
    assert(value == recovered);
  }

  static void Test_ScanfSimple() {

    for (u32 i = 1; i != 0; i <<= 1) {
      Test_PrintfScanfRoundTrip("%d", i);
      Test_PrintfScanfRoundTrip("%o", i);
      Test_PrintfScanfRoundTrip("%x", i);
      Test_PrintfScanfRoundTrip("%b", i);
      Test_PrintfScanfRoundTrip("%t", i);
      Test_PrintfScanfRoundTrip("boo%b", i);
      Test_PrintfScanfRoundTrip("%dork", i);

      Test_PrintfScanfRoundTrip("%012d", i);
      Test_PrintfScanfRoundTrip("%018o", i);
      Test_PrintfScanfRoundTrip("%024x", i);
      Test_PrintfScanfRoundTrip("%040b", i);
      Test_PrintfScanfRoundTrip("%07t", i);

      // Add tempting distractions at the end..
      Test_PrintfScanfRoundTrip("%12d9", i);
      Test_PrintfScanfRoundTrip("%18o7", i);
      Test_PrintfScanfRoundTrip("%24xF", i);
      Test_PrintfScanfRoundTrip("%40b0", i);
      Test_PrintfScanfRoundTrip("%7tZ", i);

    }

    tester.Reset("foo=12; bar= 34;");
    u32 foo = 0, bar = 0;
    assert(13 == tester.Scanf("foo=%d; bar=%d;", &foo, &bar));
    assert(foo == 12);
    assert(bar == 34);
    assert(tester.Read() < 0);

    tester.Reset();
    assert(13 == tester.Scanf("foo=%o; bar=%x;", &foo, &bar));
    assert(foo == 012);
    assert(bar == 0x34);
    assert(tester.Read() < 0);

    tester.Reset();
    assert(13 == tester.Scanf("f%co=%o; %car=%x;", &foo, 0, &bar, 0));
    assert(foo == 'o');
    assert(bar == 'b');
    assert(tester.Read() < 0);

    tester.Reset("110% pure");
    assert(7 == tester.Scanf("%d%% pure", &foo));
    assert(foo == 110);

    tester.Reset("\1\2\3\4");
    assert(1 == tester.Scanf("%c", &foo));
    assert(foo == 1);

    tester.Reset();
    assert(1 == tester.Scanf("%h", &foo));
    assert(foo == 0x0102);

    tester.Reset();
    assert(1 == tester.Scanf("%l", &foo));
    assert(foo == 0x01020304);

  }

  static void Test_ScanfComplex() {

    tester.Reset("\n\n\tFoo( 12, bar, Hi-11122348201405166191339)\n");
    CharBufferByteSink<100> funcName;

    assert(4 == tester.Scanf("%#[\t\n ]%[A-Za-z0-9]%#[\t\n ](",&funcName));
    assert(funcName.Equals("Foo"));

    u32 num;
    assert(4 == tester.Scanf("%#[\t\n ]%d%#[\t\n ],",&num));

    CharBufferByteSink<100> arg2;
    assert(4 == tester.Scanf("%#[\t\n ]%[^,)\t\n ]%#[\t\n ],",&arg2));
    assert(arg2.Equals("bar"));

    UUID arg3;
    assert(4 == tester.Scanf("%#[\t\n ]%@%#[\t\n ])",&arg3));
    assert(!strcmp(arg3.GetLabel(),"Hi"));
    //                             "(%[\t\n ]%[^,\t\n ]%[^\t\n ],
  }

  void ByteSource_Test::Test_RunTests() {
    Test_Basic();
    Test_Unread();
    Test_Scan();
    Test_ScanSetFormat();
    Test_ScanSet();
    Test_ScanFieldwidths();
    Test_ScanfSimple();
    Test_ScanfComplex();
  }

} /* namespace MFM */
