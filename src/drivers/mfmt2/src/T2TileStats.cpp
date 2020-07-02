#include "T2TileStats.h"

namespace MFM {

  void T2TileStats::reset() {
    mResetTime = UniqueTime::now();
#define XX(NM,CM) m##NM = 0;

    ALL_TILE_STAT_U64S()
#undef XX      
  }

  void T2TileStats::saveRaw(ByteSink & bs) {
    bs.Printf("%l%l",
              mResetTime.tv_sec,
              mResetTime.tv_nsec);
#define XX(NM,CM) bs.Printf("%q", get##NM());

    ALL_TILE_STAT_U64S()
#undef XX      
  }

  bool T2TileStats::loadRaw(ByteSource & bs) {
    T2TileStats tmp;
    if (2 != bs.Scanf("%l%l",
                      &tmp.mResetTime.tv_sec,
                      &tmp.mResetTime.tv_nsec))
      return false;
#define XX(NM,CM) if (1 != bs.Scanf("%q",&tmp.m##NM)) return false;

    ALL_TILE_STAT_U64S()
#undef XX

    *this = tmp;
    return true;
  }

}
