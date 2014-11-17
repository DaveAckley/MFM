#include "assert.h"
#include "GridTransceiver_Test.h"
#include "itype.h"
#include <string.h> // For strlen
#include <stdio.h> // For fprintf

namespace MFM {

  void GridTransceiver_Test::Test_Basic() {
    GridTransceiver pt;

    pt.SetEnabled(true);
    assert(pt.CanRead(true) == 0);
    assert(pt.CanRead(false) == 0);

    assert(pt.CanWrite(true) == GridTransceiver::BUFFER_SIZE - 1);
    assert(pt.CanWrite(false) == GridTransceiver::BUFFER_SIZE - 1);

    assert(pt.CanXmit(true) == 0);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 0);

    const char * aWrite = "foo";
    const u32 aLen = strlen(aWrite);
    u32 awrote;

    // 'foo' from a to b
    awrote = pt.Write(true, (const u8 *) aWrite, aLen);
    assert(awrote == aLen);

    assert(pt.CanXmit(true) == aLen);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 0);

    const char * bWrite = "barf";
    const u32 bLen = strlen(bWrite);
    u32 bwrote;

    // 'barf' from b to a
    bwrote = pt.Write(false, (const u8 *) bWrite, bLen);
    assert(bwrote == bLen);

    assert(pt.CanXmit(true) == aLen);
    assert(pt.CanXmit(false) == bLen);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 0);

    // now 'foofoo' from a to b
    awrote = pt.Write(true, (const u8 *) aWrite, aLen);
    assert(awrote == aLen);

    assert(pt.CanXmit(true) == 2*aLen);
    assert(pt.CanXmit(false) == bLen);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 0);

    // Push two bytes from the write into the air
    pt.Transceive(2, 2);

    assert(pt.CanXmit(true) == 2*aLen - 2);
    assert(pt.CanXmit(false) == bLen - 2);

    assert(pt.CanRcv(true) == 2);
    assert(pt.CanRcv(false) == 2);

    assert(pt.CanRead(true) == 0); // Nothing's arrived yet
    assert(pt.CanRead(false) == 0);

    // Push three more bytes through the system (but only two in air)
    pt.Transceive(3, 2);

    assert(pt.CanXmit(true) == 2*aLen - 2 - 3);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 2);
    assert(pt.CanRcv(false) == 2);

    assert(pt.CanRead(true) == 2);
    assert(pt.CanRead(false) == 3);

    u8 buff[8];
    u32 read;

    read = pt.Read(true, buff, 8);
    assert(read == 2);
    assert(buff[0] == 'b');
    assert(buff[1] == 'a');

    read = pt.Read(false, buff, 8);
    assert(read == 3);
    assert(buff[0] == 'f');
    assert(buff[1] == 'o');
    assert(buff[2] == 'o');

    // Push up to ten more bytes through the system
    pt.Transceive(10, 2);

    assert(pt.CanXmit(true) == 0);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0); // 1 down, none in air
    assert(pt.CanRcv(false) == 1); // 1 down, 1 in air

    assert(pt.CanRead(true) == 2);
    assert(pt.CanRead(false) == 2);

    // Push up to ten more bytes through the system
    pt.Transceive(10, 2);

    assert(pt.CanXmit(true) == 0);  // Write buffers all empty
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0); // Air is empty
    assert(pt.CanRcv(false) == 0);

    assert(pt.CanRead(true) == 2);
    assert(pt.CanRead(false) == 3);

    read = pt.Read(true, buff, 8);
    assert(read == 2);
    assert(buff[0] == 'r');
    assert(buff[1] == 'f');

    read = pt.Read(false, buff, 8);
    assert(read == 3);
    assert(buff[0] == 'f');
    assert(buff[1] == 'o');
    assert(buff[2] == 'o');

    read = pt.Read(true, buff, 8);
    assert(read == 0);

    read = pt.Read(false, buff, 8);
    assert(read == 0);
  }


#if 0
  static void drain(GridTransceiver & pt, bool side, u32 i)
  {
    u8 buff[10000];
    u32 bytes = pt.CanRead(side);
    assert(bytes < sizeof(buff));
    u32 read = pt.Read(side,buff,bytes);
    assert(read == bytes);
    buff[read] = 0;
    fprintf(stderr,"%s %d (%d) [%d]:%s\n", side?"T":"F", i, pt.CanWrite(side), read, buff);
  }

  void GridTransceiver_Test::Test_DataRates() {
    GridTransceiver pt;
    for (u32 i = 0; i < 100; ++i) {
      if (i < 80 || (i%4 == 0)) {
        pt.Write(true, (const u8*) "abcdefghijklmnopqrstuvwxyz0123456789!!!!!!!!!!",strlen("abcdefghijklmnopqrstuvwxyz0123456789!!!!!!!!!!"));
        pt.Write(false,(const u8*) "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",strlen("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
      }
      pt.Advance(333333); // 1/3 million nanos = 1/3 ms
      drain(pt,true,i);
      drain(pt,false,i);
    }
  }
#endif

  void GridTransceiver_Test::Test_DataRates() {
    GridTransceiver pt;
    assert(!pt.IsEnabled());

    assert(pt.GetDataRate() == 500000);  // default
    assert(pt.GetMaxInFlight() == 1);  // default

    pt.SetDataRate(13); // absurdly tiny bytes per second
    pt.SetMaxInFlight(2); // absurdly many of them in flight

    assert(pt.GetDataRate() == 13);
    assert(pt.GetMaxInFlight() == 2);

    pt.SetEnabled(true);
    assert(pt.IsEnabled());

    const char * data = "abcdefghijklmnopqrstuvwxyz0123456789";
    const u32 slen = strlen(data);
    const u8 * str = (const u8*) data;

    pt.Write(true, (const u8*) str,slen);

    assert(pt.CanXmit(true) == slen);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 0);

    assert(pt.CanRead(true) == 0);
    assert(pt.CanRead(false) == 0);

    const u32 ONE_BILLION = 1000 * 1000 * 1000;
    pt.Advance(ONE_BILLION);

    assert(pt.CanXmit(true) == slen - 13);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 2);

    assert(pt.CanRead(true) == 0);
    assert(pt.CanRead(false) == 11);

    pt.Advance(ONE_BILLION);

    assert(pt.CanXmit(true) == slen - 2 * 13);
    assert(pt.CanXmit(false) == 0);

    assert(pt.CanRcv(true) == 0);
    assert(pt.CanRcv(false) == 2);

    assert(pt.CanRead(true) == 0);
    assert(pt.CanRead(false) == 13 + 11);
  }

  void GridTransceiver_Test::Test_RunTests() {
    Test_Basic();
    Test_DataRates();
  }

} /* namespace MFM */
