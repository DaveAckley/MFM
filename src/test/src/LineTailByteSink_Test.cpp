#include "assert.h"
#include "LineTailByteSink_Test.h"
#include "LineTailByteSink.h"
#include <stdlib.h>        /* For strtol */

namespace MFM {
  typedef LineTailByteSink<3,10> OCBS3x10;
  typedef LineTailByteSink<10,5> OCBS10x5;

  static OCBS3x10 buf3x10;
  static OCBS10x5 buf10x5;

  static void Test_Overflows() {
    buf3x10.Reset();
    assert(buf3x10.GetLines()==1);
    assert(!strcmp("",buf3x10.GetZString(0)));
    assert(!buf3x10.GetZString(1));
    assert(!buf3x10.GetZString(2));
    assert(!buf3x10.GetZString(3));
    assert(!buf3x10.GetZString(10));

    buf3x10.Print("x");
    assert(buf3x10.GetLines()==1);
    assert(!strcmp("x",buf3x10.GetZString(0)));

    buf3x10.Print("y\nabc");
    assert(buf3x10.GetLines()==2);
    assert(!strcmp("xy",buf3x10.GetZString(0)));
    assert(!strcmp("abc",buf3x10.GetZString(1)));

    buf3x10.Print("\n");
    assert(buf3x10.GetLines()==3);
    assert(!strcmp("xy",buf3x10.GetZString(0)));
    assert(!strcmp("abc",buf3x10.GetZString(1)));
    assert(!strcmp("",buf3x10.GetZString(2)));
    assert(!buf3x10.GetZString(3));
    assert(!buf3x10.GetZString(333));

    buf3x10.Print("0123");
    assert(buf3x10.GetLines()==3);
    assert(!strcmp("xy",buf3x10.GetZString(0)));
    assert(!strcmp("abc",buf3x10.GetZString(1)));
    assert(!strcmp("0123",buf3x10.GetZString(2)));

    buf3x10.Print("456789");
    assert(buf3x10.GetLines()==3);
    assert(!strcmp("xy",buf3x10.GetZString(0)));
    assert(!strcmp("abc",buf3x10.GetZString(1)));
    assert(!strcmp("01234567X",buf3x10.GetZString(2)));

    buf3x10.Print("dumped");
    assert(buf3x10.GetLines()==3);
    assert(!strcmp("xy",buf3x10.GetZString(0)));
    assert(!strcmp("abc",buf3x10.GetZString(1)));
    assert(!strcmp("01234567X",buf3x10.GetZString(2)));

    buf3x10.Print("\n");
    assert(buf3x10.GetLines()==3);
    assert(!strcmp("abc",buf3x10.GetZString(0)));
    assert(!strcmp("01234567X",buf3x10.GetZString(1)));
    assert(!strcmp("",buf3x10.GetZString(2)));
    assert(!buf3x10.GetZString(3));

    buf3x10.Print("f");
    assert(buf3x10.GetLines()==3);
    assert(!strcmp("abc",buf3x10.GetZString(0)));
    assert(!strcmp("01234567X",buf3x10.GetZString(1)));
    assert(!strcmp("f",buf3x10.GetZString(2)));

  }

  static void Test_Trims() {
    buf10x5.Reset();
    assert(buf10x5.GetLines()==1);
    assert(!strcmp("",buf10x5.GetZString(0)));
    assert(!buf10x5.GetZString(1));

    buf10x5.Print("0\n1\n2\n3\n4\n5\n6\n7\n8\n");
    assert(buf10x5.GetLines()==10);
    assert(!strcmp("0",buf10x5.GetZString(0)));
    assert(!strcmp("1",buf10x5.GetZString(1)));
    assert(!strcmp("2",buf10x5.GetZString(2)));
    assert(!strcmp("3",buf10x5.GetZString(3)));
    assert(!strcmp("4",buf10x5.GetZString(4)));
    assert(!strcmp("5",buf10x5.GetZString(5)));
    assert(!strcmp("6",buf10x5.GetZString(6)));
    assert(!strcmp("7",buf10x5.GetZString(7)));
    assert(!strcmp("8",buf10x5.GetZString(8)));
    assert(!strcmp("",buf10x5.GetZString(9)));
    assert(!buf10x5.GetZString(10));
    assert(!buf10x5.GetZString(11));

    buf10x5.Trim(0);
    assert(buf10x5.GetLines()==10);
    assert(!strcmp("0",buf10x5.GetZString(0)));
    assert(!buf10x5.GetZString(10));

    buf10x5.Trim(1);
    assert(buf10x5.GetLines()==9);
    assert(!strcmp("1",buf10x5.GetZString(0)));
    assert(buf10x5.GetZString(8));
    assert(!buf10x5.GetZString(9));
    assert(!buf10x5.GetZString(10));

    buf10x5.Trim(4);
    assert(buf10x5.GetLines()==5);
    assert(!strcmp("5",buf10x5.GetZString(0)));
    assert(!strcmp("6",buf10x5.GetZString(1)));
    assert(!strcmp("7",buf10x5.GetZString(2)));
    assert(!strcmp("8",buf10x5.GetZString(3)));
    assert(!strcmp("",buf10x5.GetZString(4)));
    assert(!buf10x5.GetZString(5));
    assert(!buf10x5.GetZString(6));
    assert(!buf10x5.GetZString(9));
    assert(!buf10x5.GetZString(10));

    buf10x5.Print("nine\n");
    assert(buf10x5.GetLines()==6);
    assert(!strcmp("5",buf10x5.GetZString(0)));
    assert(!strcmp("ninX",buf10x5.GetZString(4)));
    assert(!strcmp("",buf10x5.GetZString(5)));
    assert(!buf10x5.GetZString(6));

    buf10x5.Trim(4);
    assert(buf10x5.GetLines()==2);
    assert(!strcmp("ninX",buf10x5.GetZString(0)));
    assert(buf10x5.GetZString(1));
    assert(!buf10x5.GetZString(2));

    buf10x5.Trim(4);
    assert(buf10x5.GetLines()==1);
    assert(!strcmp("",buf10x5.GetZString(0)));
    assert(!buf10x5.GetZString(1));
    assert(!buf10x5.GetZString(2));

    buf10x5.Trim(4);
    assert(buf10x5.GetLines()==1);
    assert(!strcmp("",buf10x5.GetZString(0)));
    assert(!buf10x5.GetZString(1));
    assert(!buf10x5.GetZString(2));

  }

  void LineTailByteSink_Test::Test_RunTests() {
    Test_Overflows();
    Test_Trims();
  }

} /* namespace MFM */

