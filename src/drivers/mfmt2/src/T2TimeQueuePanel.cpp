#include "T2TimeQueuePanel.h"

#include "TimeQueue.h"
#include "T2Utils.h"
#include "SimLogPanel.h"
#include "T2Tile.h"

namespace MFM {
  void T2TimeQueuePanel::configure(SDLI & sdli) {
    /* Nothing to do so far */
  }

  void T2TimeQueuePanel::onTimeout(TimeQueue& srcTQ) {
    
    insert(srcTQ,1024);         // Otherwise update at about ~1HZ

    ResettableByteSink & rbs = this->GetByteSink();
    rbs.Reset();
    srcTQ.dumpQueue(rbs);
  }

}
