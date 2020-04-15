#include "T2ITC.h"
#include "T2Tile.h"

#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>    // For close
#include <stdio.h>     // For snprintf
#include <errno.h>     // For errno

namespace MFM {

  T2ITC::T2ITC(T2Tile& tile, Dir6 dir6, const char * name)
    : mTile(tile)
    , mDir6(dir6)
    , mDir8(mapDir6ToDir8(dir6))
    , mName(name)
    , mActiveFreeCount(CIRCUIT_COUNT)
    , mFD(-1)
  {
    for (u8 i = 0; i < CIRCUIT_COUNT; ++i) {
      mActiveFree[i] = i;
      for (u8 act = 0; act <= 1; ++act) {
        mCircuits[act][i].mNumber = i;
        mCircuits[act][i].mEW = 0;  // Unassigned
      }
    }
  }

  CircuitNum T2ITC::tryAllocateActiveCircuit() {
    if (mActiveFreeCount==0) return ALL_CIRCUITS_BUSY;
    Random & r = mTile.getRandom();
    u8 idx = r.Between(0,mActiveFreeCount-1);
    CircuitNum ret = mActiveFree[idx];
    mActiveFree[idx] = mActiveFree[--mActiveFreeCount];
    return ret;
  }

  void T2ITC::freeActiveCircuit(CircuitNum cn) {
    assert(cn < CIRCUIT_COUNT);
    assert(mActiveFreeCount < CIRCUIT_COUNT);
    mActiveFree[mActiveFreeCount++] = cn;
    mCircuits[1][cn].mEW = 0;
  }

  const char * T2ITC::path() const {
    static char buf[100];
    snprintf(buf,100,"/dev/itc/mfm/%s",mName);
    return buf;
  }

  int T2ITC::open() {
    int ret = ::open(path(),O_RDWR|O_NONBLOCK);
    if (ret < 0) return -errno;
    mFD = ret;
    return ret;
  }

  int T2ITC::close() {
    int ret = ::close(mFD);
    mFD = -1;
    if (ret < 0) return -errno;
    return ret;
  }
}
