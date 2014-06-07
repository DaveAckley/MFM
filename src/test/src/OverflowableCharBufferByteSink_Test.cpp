#include "assert.h"
#include "OverflowableCharBufferByteSink_Test.h"
#include "OverflowableCharBufferByteSink.h"
#include <stdlib.h>        /* For strtol */

namespace MFM {
  typedef OverflowableCharBufferByteSink<10> OCBS10;
  typedef OverflowableCharBufferByteSink<20> OCBS20;

  static OCBS10 buf10;
  static OCBS20 buf20;

  static void Test_Overflows() {
    buf10.Reset();
    assert(buf10.Equals(""));

    buf10.Reset();
    buf10.Print(10);
    assert(buf10.Equals("10"));

    buf10.Reset();
    buf10.Print(19);
    assert(buf10.Equals("19"));

    buf10.Reset();
    buf10.Print("01234567");
    assert(buf10.Equals("01234567"));

    buf10.Print("8");
    assert(buf10.Equals("01234567X"));

    buf10.Print("snorg gobble");
    assert(buf10.Equals("01234567X"));

    buf10.Print("clams");
    assert(buf10.Equals("01234567X"));

    buf10.Reset();
    buf10.Print("Blow it in one go man");
    assert(buf10.Equals("Blow it X"));

    buf20.Reset();
    assert(buf20.Equals(""));

    buf20.Print("0123456789");
    assert(buf20.Equals("0123456789"));

    buf20.Print("Blew it in one go man");
    assert(buf20.Equals("0123456789Blew it X"));
  }

  void OverflowableCharBufferByteSink_Test::Test_RunTests() {
    Test_Overflows();
  }

} /* namespace MFM */

