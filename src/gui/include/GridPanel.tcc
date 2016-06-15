/* -*- C++ -*- */

namespace MFM {

  template <class GC>
  const char * GridPanel<GC>::GetEventHistoryStrategyName(EventHistoryStrategy t)
  {
    switch (t)
    {
    default:
      FAIL(ILLEGAL_ARGUMENT);
    case EVENT_HISTORY_STRATEGY_NONE:      return "Off";
    case EVENT_HISTORY_STRATEGY_SELECTED:  return "Tile";
    case EVENT_HISTORY_STRATEGY_ALL:       return "On";
    }
  }
}
