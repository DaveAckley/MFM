#include "assert.h"
#include "Logger_Test.h"
#include "CharBufferByteSink.h"
#include <stdlib.h>        /* For strtol */

namespace MFM {
  typedef CharBufferByteSink<1024> CBS1K;

  static CBS1K tbuf;

  static void Test_Basic() {
    {
      tbuf.Reset();
      Logger log(tbuf,Logger::MESSAGE);
      log.Debug("%d captains: %s vs %s", 2, "Scarlet", "Kirk");
      log.Message("This is %s", "Captain Black");
      log.Warning("We know that you can %s us, %s","hear","Earthman");
      log.Error("Must sterilize");
      assert(!strcmp("11: MSG: This is Captain Black\n12: WRN: We know that you can hear us, Earthman\n13: ERR: Must sterilize\n",
                     tbuf.GetZString()));
    }
    {
      tbuf.Reset();
      Logger log(tbuf,Logger::DEBUG);
      log.Debug("%d captains: %s vs %s", 2, "Scarlet", "Kirk");
      log.Message("This is %s", "Captain Black");
      log.Warning("We know that you can %s us, %s","hear","Earthman");
      log.Error("Must sterilize");
      assert(!strcmp("11: DBG: 2 captains: Scarlet vs Kirk\n12: MSG: This is Captain Black\n13: WRN: We know that you can hear us, Earthman\n14: ERR: Must sterilize\n",
                     tbuf.GetZString()));
    }
    {
      tbuf.Reset();
      Logger log(tbuf,Logger::ERROR);
      log.Debug("%d captains: %s vs %s", 2, "Scarlet", "Kirk");
      log.Message("This is %s", "Captain Black");
      log.Warning("We know that you can %s us, %s","hear","Earthman");
      log.Error("Must sterilize");
      assert(!strcmp("11: ERR: Must sterilize\n",
                     tbuf.GetZString()));
    }
    {
      tbuf.Reset();
      Logger log(tbuf,Logger::DEBUG);
      log.Debug("%d captains: %s vs %s", 2, "Scarlet", "Kirk");

      Logger::Level old = log.SetLevel(Logger::WARNING);
      assert(Logger::DEBUG==old);

      log.Message("This is %s", "Captain Black");
      log.Warning("We know that you can %s us, %s","hear","Earthman");
      log.Error("Must sterilize");
      assert(!strcmp("11: DBG: 2 captains: Scarlet vs Kirk\n"
                     "12: DBG: [DBG->WRN]\n"  // Sat Sep 20 06:11:26 2014 Log level changes demoted to DBG
                     "13: WRN: We know that you can hear us, Earthman\n"
                     "14: ERR: Must sterilize\n",
                     tbuf.GetZString()));
    }
  }

  static void Test_IfLog() {
    {
      tbuf.Reset();
      Logger log(tbuf,Logger::MESSAGE);
      log.SetTimeStamper(&NullSerializable);

      for (Logger::Level i = log.ERROR; i <= log.DEBUG; i = (Logger::Level) (i+1)) {
        log.Error("@%s",Logger::StrLevel(log.GetLevel()));
        log.SetLevel(i);
        for (Logger::Level j = log.ERROR; j <= log.DEBUG; j = (Logger::Level) (j+1)) {
          log.IfLog(j) && log.Log(j,"at %s", Logger::StrLevel((Logger::Level) i));
        }
      }
      assert(!strcmp("ERR: @MSG\nERR: at ERR\n"
                     "ERR: @ERR\nERR: at WRN\nWRN: at WRN\n"
                     "ERR: @WRN\nERR: at MSG\nWRN: at MSG\nMSG: at MSG\n"
                     "ERR: @MSG\nDBG: [DBG]\nERR: at DBG\nWRN: at DBG\nMSG: at DBG\nDBG: at DBG\n",
                     tbuf.GetZString()));
    }
  }

  void Logger_Test::Test_RunTests() {
    Test_Basic();
    Test_IfLog();
  }

} /* namespace MFM */

