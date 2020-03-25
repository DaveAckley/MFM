/* -*- C++ -*- */
#ifndef T2TILE_H
#define T2TILE_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"

//Spike files
#include "T2ITC.h"
#include "EWSet.h"

#define MAX_EWSLOT 32

namespace MFM {
  struct T2Tile {

    T2Tile(u32 width, u32 height) ;
    ~T2Tile() ;
    
    void error(const char * fmt, ...) ;
    bool openITCs();
    void closeITCs();
    const u32 mWidth, mHeight;

    T2ITC mITCs[DIR6_COUNT];

    T2EventWindow * allocEW() ;
    void freeEW(T2EventWindow * ew) ;

    void setActive(T2EventWindow * ew) ;
    void setPassive(T2EventWindow * ew) ;

    EWSet mFree;
    EWSet mActive;
    EWSet mPassive;
  };
}
#endif /* T2TILE_H */
