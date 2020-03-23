/* -*- C++ -*- */
#ifndef TILE_H
#define TILE_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"

//Spike files
#include "ITC12.h"
#include "EWSet12.h"

#define MAX_EWSLOT 32

namespace MFM {
  struct Tile {

    Tile(u32 width, u32 height) ;
    ~Tile() ;
    
    void error(const char * fmt, ...) ;
    bool openITCs();
    void closeITCs();
    const u32 mWidth, mHeight;

    ITC mITCs[DIR6_COUNT];

    EventWindow * allocEW() ;
    void freeEW(EventWindow * ew) ;

    void setActive(EventWindow * ew) ;
    void setPassive(EventWindow * ew) ;

    EWSet mFree;
    EWSet mActive;
    EWSet mPassive;
  };
}
#endif /* TILE_H */
