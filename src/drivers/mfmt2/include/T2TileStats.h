/* -*- C++ -*- */
#ifndef T2TILESTATS_H
#define T2TILESTATS_H

#include "UniqueTime.h"

#define ALL_TILE_STAT_U64S()                                       \
  XX(EventsConsidered,"whether or not an aEW launched")            \
  XX(EmptyEventsCommitted,"by doing nothing")                      \
  XX(NonemptyEventsStarted,"#aEWs launched, w or w/o transition")  \
  XX(NonemptyTransitionsStarted,"#executeEvent entries")           \
  XX(NonemptyEventsCommitted,"w/o failure during transition")      \

namespace MFM {
  struct T2TileStats {
    void saveRaw(ByteSink& bs) ;
    bool loadRaw(ByteSource& bs) ;
    void reset() ;

    struct timespec mResetTime; // When stats were last reset
#define XX(NM,CM)                         \
    u64 m##NM; /* CM */                   \
    u64 get##NM() const { return m##NM; } \
    void incr##NM() { ++m##NM; }          \

    ALL_TILE_STAT_U64S()
#undef XX      
  };

}

#endif /* T2TILESTATS_H */
