/* -*- C++ -*- */
#ifndef ITC_H
#define ITC_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"

// LKM files
#include "itcpktevent.h"

// Spike files

typedef __u32 Dir8;
typedef __u32 Dir6;

namespace MFM {
  struct Tile; // FORWARD
  struct ITC {
  ITC(Tile& tile, Dir6 dir6, const char * name)
      : mTile(tile)
      , mDir6(dir6)
      , mDir8(mapDir6ToDir8(dir6))
      , mName(name)
      , mFD(-1)
    { }
    
    Tile & mTile;
    const Dir6 mDir6;
    const Dir8 mDir8;
    const char * mName;
    int mFD;

    const char * path() const;
    int open() ;
    int close() ;
    int getFD() const { return mFD; }
  };
}
#endif /* ITC_H */
