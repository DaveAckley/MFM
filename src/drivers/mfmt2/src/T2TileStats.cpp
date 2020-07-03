#include "T2TileStats.h"

namespace MFM {

  void T2TileStats::reset() {
    mResetTime = UniqueTime::now();
#define XX(NM,CM) m##NM = 0;

    ALL_TILE_STAT_U64S()
#undef XX      
  }

  void T2TileStats::saveRaw(ByteSink & bs) {
    struct timespec now = UniqueTime::now();
    bs.Printf("%l", now.tv_sec - mResetTime.tv_sec);
#define XX(NM,CM) bs.Printf("%q", get##NM());

    ALL_TILE_STAT_U64S()
#undef XX      
  }

  bool T2TileStats::loadRaw(ByteSource & bs) {
    T2TileStats tmp;
    if (1 != bs.Scanf("%l",
                      &tmp.mResetTime.tv_sec))
      return false;
    tmp.mResetTime.tv_nsec = 0;
#define XX(NM,CM) if (1 != bs.Scanf("%q",&tmp.m##NM)) return false;

    ALL_TILE_STAT_U64S()
#undef XX

    *this = tmp;
    return true;
  }

}
