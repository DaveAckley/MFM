#include "T2DrawConfigPanel.h"

#include "TimeQueue.h"
#include "T2Utils.h"
#include "SimLogPanel.h"
#include "T2Tile.h"

namespace MFM {
  void T2DrawConfigPanel::configure(SDLI & sdli) {
    /* Nothing to do so far */
  }

  void T2DrawConfigPanel::onTimeout(TimeQueue& srcTQ) {
    T2Tile & tile = T2Tile::get();
    const SiteRenderConfig & src = tile.getSiteRenderConfig();
    
    ResettableByteSink & rbs = this->GetByteSink();
    const char * typenames[] =
      {
       DrawPanelManager::getSuffixFromDrawSiteType(src.mBackType),
       DrawPanelManager::getSuffixFromDrawSiteType(src.mMidType),
       DrawPanelManager::getSuffixFromDrawSiteType(src.mFrontType)
      };
    const int FLEN=11;
    char buf[50];
    rbs.Reset();
    for (u32 i = 0; i < sizeof(typenames)/sizeof(typenames[0]);++i) {
      const char * type = typenames[i];
      int blen = strlen(type);
      int lside = (FLEN-blen)/2;
      int rside = FLEN-blen-lside;
      snprintf(buf,50,"%*s%s%*s",
               lside,"",
               typenames[i],
               rside,"");
      rbs.Printf("%s",buf);
    }
    rbs.Printf("\n\n\n\n\n\n\n\n\n\n\n");
    insert(srcTQ,1024);         // Otherwise update at about ~1HZ

    /*
    ResettableByteSink & rbs = this->GetByteSink();
    rbs.Reset();
    srcTQ.dumpQueue(rbs);
    */
  }

}
