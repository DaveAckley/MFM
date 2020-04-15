/* -*- C++ -*- */
#ifndef T2ITC_H
#define T2ITC_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"

// LKM files
#include "itcpktevent.h"

// Spike files
#include "T2Types.h"

#define CIRCUIT_BITS 4   /* each for active and passive */
#define CIRCUIT_COUNT (1<<CIRCUIT_BITS)
#define ALL_CIRCUITS_BUSY 0xff

namespace MFM {
  struct T2Tile; // FORWARD
  struct T2EventWindow; // FORWARD

  struct Circuit {
    CircuitNum mNumber; // 0..CIRCUIT_COUNT-1
    u8 mEW;     // 1..MAX_EWSLOTS
  };

  struct T2ITC {
    T2ITC(T2Tile& tile, Dir6 dir6, const char * name) ;

    CircuitNum tryAllocateActiveCircuit() ;

    void freeActiveCircuit(CircuitNum cn) ;
    
    T2Tile & mTile;
    const Dir6 mDir6;
    const Dir8 mDir8;
    const char * mName;
    Circuit mCircuits[2][CIRCUIT_COUNT]; // [0][] passive, [1][] active

    u8 mActiveFreeCount;
    CircuitNum mActiveFree[CIRCUIT_COUNT];

    int mFD;
    const char * path() const;
    int open() ;
    int close() ;
    int getFD() const { return mFD; }
  };
}
#endif /* T2ITC_H */
