#include "assert.h"
#include "ByteSink_Test.h"
#include "Util.h"
#include "CharBufferByteSink.h"
#include "ByteSerializable.h"
#include <stdlib.h>        /* For strtol */

namespace MFM {
  typedef CharBufferByteSink<1024> CBS1K;

  static CBS1K tbuf;

  static void Test_Numbers() {
    tbuf.Reset();
    assert(tbuf.Equals(""));

    tbuf.Reset();
    tbuf.Print(10);
    assert(tbuf.Equals("10"));

    tbuf.Reset();
    tbuf.Print(19);
    assert(tbuf.Equals("19"));

    tbuf.Reset();
    tbuf.Print(19, Format::DEC);
    assert(tbuf.Equals("19"));

    tbuf.Reset();
    tbuf.Print(19, Format::OCT);
    assert(tbuf.Equals("23"));

    tbuf.Reset();
    tbuf.Print(19, Format::BIN);
    assert(tbuf.Equals("10011"));

    tbuf.Reset();
    tbuf.Print(10, Format::HEX);
    assert(tbuf.Equals("A"));

    tbuf.Reset();
    tbuf.Print(0x10, Format::HEX);
    assert(tbuf.Equals("10"));

    tbuf.Reset();
    tbuf.Print(0xdeadcafe, Format::HEX);
    assert(tbuf.Equals("DEADCAFE"));

    tbuf.Reset();
    tbuf.Print(36, Format::B36);
    assert(tbuf.Equals("10"));

    tbuf.Reset();
    tbuf.Print(-1, Format::B36);
    assert(tbuf.Equals("1Z141Z3"));
  }

  static void Test_Bytes() {

    tbuf.Reset();
    tbuf.Print(19, Format::BYTE);
    assert(tbuf.Equals("\023"));

    tbuf.Reset();
    tbuf.Print(0x0102, Format::BEU16);
    assert(tbuf.Equals("\001\002"));

    tbuf.Reset();
    tbuf.Print(0xfffe, Format::BEU16);
    assert(tbuf.Equals("\377\376"));

    tbuf.Reset();
    tbuf.Print(0x04050607, Format::BEU32);
    assert(tbuf.Equals("\004\005\006\007"));

    tbuf.Reset();
    tbuf.Print(HexU64(0x04050607,0x08090a0b), Format::BEU64);
    assert(tbuf.Equals("\004\005\006\007\010\011\012\013"));

    tbuf.Reset();
    tbuf.Print(HexU64(0xfeedface,0xdeadbeef), Format::BEU64);
    assert(tbuf.Equals("\376\355\372\316\336\255\276\357"));

  }

  static void Test_Lex() {

    tbuf.Reset();
    tbuf.Print(19, Format::LEX32);
    assert(tbuf.Equals("219"));

    tbuf.Reset();
    tbuf.Print(0x1234, Format::LXX32);
    assert(tbuf.Equals("41234"));

    tbuf.Reset();
    tbuf.Print(-1, Format::LXX32);
    assert(tbuf.Equals("8FFFFFFFF"));

    tbuf.Reset();
    tbuf.Print(DecU64(123456,789101112), Format::LEX64);
    assert(tbuf.Equals("9215123456789101112"));

    tbuf.Reset();
    tbuf.Print(HexU64(0xfeedface,0xdeadbeef), Format::LXX64);
    assert(tbuf.Equals("9216FEEDFACEDEADBEEF"));

  }

  static void Test_Vprintf_Result(const char * result, const char * format, ... ) {
    va_list ap;
    tbuf.Reset();
    va_start(ap, format);
    tbuf.Vprintf(format, ap);
    va_end(ap);
    assert(tbuf.Equals(result));
  }

  static void Test_Printf() {
    Test_Vprintf_Result("Hello world", "Hello world");
    Test_Vprintf_Result("Hello world\n", "Hello world\n");

    Test_Vprintf_Result("100% spec\n", "100%% spec\n");
    Test_Vprintf_Result("%110 spec\n", "%%%d spec\n",110);

    Test_Vprintf_Result("foo=123\n", "foo=%d\n",123);
    Test_Vprintf_Result("foo=FACE\n", "foo=%x\n",0xface);
    Test_Vprintf_Result("foo=10\n", "foo=%o\n",8);
    Test_Vprintf_Result("foo=Z00\n", "foo=%t\n",35*36*36);

    Test_Vprintf_Result("foo=\020\021\n", "foo=%h\n",0x1011);
    Test_Vprintf_Result("foo=\020\021\022\023\n", "foo=%l\n",0x10111213);
    Test_Vprintf_Result("foo=\020\021\022\023\024\025\026\027\n", "foo=%q\n",HexU64(0x10111213,0x14151617));

    Test_Vprintf_Result("%d", "%c%c",'%','d');
    Test_Vprintf_Result("foo=bar=gah", "foo=%s%sh","bar","=ga");

    Test_Vprintf_Result("foo=(null)", "foo=%s",(const char *) 0);

    Test_Vprintf_Result("(0)", "(%H)",0);
    Test_Vprintf_Result("(1)", "(%H)",1);
    Test_Vprintf_Result("(8)", "(%H)",8);
    Test_Vprintf_Result("(919)", "(%H)",9);
    Test_Vprintf_Result("(9210)", "(%H)",10);
    Test_Vprintf_Result("(9211)", "(%H)",11);
    Test_Vprintf_Result("(93123)", "(%H)",123);
    Test_Vprintf_Result("(941234)", "(%H)",1234);
    Test_Vprintf_Result("(9512345)", "(%H)",12345);
    Test_Vprintf_Result("(96123456)", "(%H)",123456);
    Test_Vprintf_Result("(971234567)", "(%H)",1234567);
    Test_Vprintf_Result("(9812345678)", "(%H)",12345678);
    Test_Vprintf_Result("(9919123456789)", "(%H)",123456789);
    Test_Vprintf_Result("(992101234567890)", "(%H)",1234567890);

    Test_Vprintf_Result("(nullp)","%p",(void*) 0);
    {
      char a;
      char b;
      tbuf.Reset();
      tbuf.Printf("%p",&a);

      const char * z = tbuf.GetZString();
      char * ez;
      uptr ua = strtol(z, &ez, 16);
      assert(*z != 0 && *ez == 0);   // Ate whole string
      assert(&a == (void*) ua);      // Recovered an identical pointer
      assert(&b != (void*) ua);      // Different from some other pointer
    }
  }

  static void Test_PrintfWidths() {
    Test_Vprintf_Result("  1","%3d",1);
    Test_Vprintf_Result("002","%03d",2);
    Test_Vprintf_Result("-03","%03d",-3);
  }

  class IPrintMyself : public ByteSerializable {
  public:
    IPrintMyself(const char * str) : m_str(str) { }

    Result PrintTo(ByteSink & bs, s32 arg = 0) {
      bs.Printf("[IPM:%s=%d]",m_str,arg);
      return SUCCESS;
    }

    Result ReadFrom(ByteSource & source, s32 arg = 0)
    {
      return UNSUPPORTED;
    }
  private:
    const char * m_str;
  };


  static void Test_Sinkable() {
    IPrintMyself p1("wow"), p2("modern");
    Test_Vprintf_Result("so [IPM:wow=0], such [IPM:modern=3]!",
                        "so %@, such %#@!",&p1,3,&p2);
    Test_Vprintf_Result("so (null), such [IPM:modern=3]!",
                        "so %@, such %#@!",(void*)0,3,&p2);
    Test_Vprintf_Result("so [IPM:wow=0], such (null)!",
                        "so %@, such %#@!",&p1,3,(void*)0);

  }

  void ByteSink_Test::Test_RunTests() {
    Test_Numbers();
    Test_Bytes();
    Test_Lex();
    Test_Printf();
    Test_PrintfWidths();
    Test_Sinkable();
  }

} /* namespace MFM */

