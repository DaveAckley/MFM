#include "EventHistoryItem.h"

namespace MFM {

  void EventHistoryItem::Print(ByteSink & bs) const
  {
    bs.Printf("[EHI-%s",GetTypeName());
    if (IsHeader())
    {
      bs.Printf(",count=%d,site=(%d,%d),event#=%d",
                mHeaderItem.m_count,
                mHeaderItem.m_siteInTileX,
                mHeaderItem.m_siteInTileY,
                mHeaderItem.m_eventNumber);
                
    }
    else if (IsDelta())
    {
      bs.Printf(",#=%d,code=%d,word=%d,old=%08x,new=%08x",
                mDeltaItem.m_count,
                mDeltaItem.m_site,
                mDeltaItem.m_word,
                mDeltaItem.m_oldValue,
                mDeltaItem.m_newValue);
    }
    bs.Printf("]");
  }
}
