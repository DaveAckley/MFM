#include "T2TileStats.h"
#include "T2Constants.h"
#include <cmath> /*for nan() */

namespace MFM {

  void T2TileStats::reset() {
    mResetSeconds = UniqueTime::now().tv_sec;
#define XX(NM,CM) m##NM = 0;

    ALL_TILE_STAT_U64S()
#undef XX      
  }

  u32 T2TileStats::getAgeSeconds() const {
    return UniqueTime::now().tv_sec - mResetSeconds;
  }

  void T2TileStats::saveRaw(ByteSink & bs) {
    struct timespec now = UniqueTime::now();
    bs.Printf("%l", now.tv_sec - mResetSeconds);
#define XX(NM,CM) bs.Printf("%q", get##NM());

    ALL_TILE_STAT_U64S()
#undef XX      
  }

  bool T2TileStats::loadRaw(ByteSource & bs) {
    T2TileStats tmp;
    if (1 != bs.Scanf("%l",
                      &tmp.mResetSeconds))
      return false;
#define XX(NM,CM) if (1 != bs.Scanf("%q",&tmp.m##NM)) return false;

    ALL_TILE_STAT_U64S()
#undef XX

    *this = tmp;
    return true;
  }

  T2TileStats T2TileStats::operator-(const T2TileStats & rhs) {
    T2TileStats ret;
    u32 now = UniqueTime::now().tv_sec;
    u32 agelhs = now - getResetSeconds();        // Time we've been capturing
    u32 agerhs = now - rhs.getResetSeconds();    // Time they've been capturing
    ret.mResetSeconds = now - (agelhs - agerhs); // Time ago our difference started

#define XX(NM,CM) ret.m##NM = get##NM() - rhs.get##NM();

    ALL_TILE_STAT_U64S()
#undef XX      
    return ret;
  }

  double T2TileStats::getEstAER(u32 secsDuration) const {
    u32 secs;
    if (secsDuration != 0) secs = secsDuration;
    else {
      u32 now = UniqueTime::now().tv_sec;
      if (mResetSeconds >= now) return nan("");
      secs = now - mResetSeconds;
    }
    double estAER =
      ((double) (getEmptyEventsCommitted() +
                 getNonemptyEventsCommitted())) / secs / (T2TILE_OWNED_WIDTH * T2TILE_OWNED_HEIGHT);
    return estAER;
  }



}
