#include "T2CDMPanel.h"

#include "TimeQueue.h"
#include "T2Utils.h"
#include "T2Tile.h"

namespace MFM {
  void T2CDMPanel::configure(SDLI & sdli) {
    /* Nothing to do so far */
  }

  static void reportCDMInfo(ByteSink & bs) {
    static u32 prevmodtime;
    static u32 lastchangems;
    T2Main & m = T2Main::get();
    u32 nowms = m.now();
    const char * PATH = "/cdm/log/status.txt";  // "/home/t2/T2-12/apps/cdm/cdm/cdmDEBUG/log/status.txt"
    u32 curmodtime = getModTimeOfFile(PATH);
    const u32 DOWN_TIME_SEC = 20;
    if (curmodtime != prevmodtime) {
      prevmodtime = curmodtime;
      lastchangems = nowms;
    }
    u32 diffms = nowms - lastchangems;
    if (diffms > DOWN_TIME_SEC*1000) {
      bs.Printf("CDM STATUS: DOWN?\n");
      return;
    }
    if (!readWholeFile(PATH,bs)) {
      bs.Printf("CDM STATUS: UNAVAILABLE\n");
    }
  }

  void T2CDMPanel::onTimeout(TimeQueue& srcTQ) {
    
    insert(srcTQ,1024);         // Otherwise update at about ~1HZ

    ResettableByteSink & rbs = this->GetByteSink();
    rbs.Reset();
    reportCDMInfo(rbs);
  }

}
