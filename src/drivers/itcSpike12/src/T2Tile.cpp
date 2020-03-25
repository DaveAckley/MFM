#include "T2Tile.h"

#include <stdio.h>   // For printf
#include <string.h>  // For strerror
#include <stdarg.h>  // For va_args

// Spike files
#include "T2EventWindow.h"

#define ALL_DIR6_MACRO() XX(ET)YY XX(SE)YY XX(SW)YY XX(WT)YY XX(NW)YY XX(NE)ZZ

namespace MFM {

  T2Tile::T2Tile(u32 wid, u32 hei)
    : mWidth(wid)
    , mHeight(hei)
    , mITCs{
#define XX(dir6) T2ITC(*this,DIR6_##dir6,#dir6)
#define YY ,
#define ZZ
        ALL_DIR6_MACRO()
#undef XX
#undef YY
#undef ZZ
      }
    , mFree(*this)
    , mActive(*this)
    , mPassive(*this)
  {
    for (u8 i = 1; i <= MAX_EWSLOT; ++i) {
      mFree.pushBack(new T2EventWindow(*this, i));
    }
  }

  T2Tile::~T2Tile() { closeITCs(); }

  bool T2Tile::openITCs() {
    int failed = 0;
    for (int i = 0; i < DIR6_COUNT; ++i) {
      T2ITC & itc = mITCs[i];
      int err = itc.open();
      if (err < 0) 
        error("openITC error %d: Could not open %s: %s\n",
              ++failed,
              itc.path(),
              strerror(-err));
    }
    return failed == 0;
  }

  void T2Tile::closeITCs() {
    for (int i = 0; i < DIR6_COUNT; ++i) {
      T2ITC & itc = mITCs[i];
      itc.close();
    }
  }

  void T2Tile::error(const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vfprintf(stderr,msg,ap);
    va_end(ap);
  }

  T2EventWindow * T2Tile::allocEW() {
    return mFree.pop(); // 0 if none
  }

  void T2Tile::freeEW(T2EventWindow * ew) {
    assert(ew!=0);
    mFree.pushBack(ew);
  }

  void T2Tile::setActive(T2EventWindow * ew) {
    assert(ew!=0);
    mActive.pushBack(ew);
  }
  
  void T2Tile::setPassive(T2EventWindow * ew) {
    assert(ew!=0);
    mPassive.pushBack(ew);
  }

}
