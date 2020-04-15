/* -*- C++ -*- */
#ifndef T2TILE_H
#define T2TILE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Random.h"
#include "P3Atom.h"
#include "MDist.h"   /* For site number <-> SPoint mapping */

//Spike files
#include "T2Constants.h"
#include "T2Types.h"
#include "T2ITC.h"
#include "EWSet.h"
#include "TimeQueue.h"
#include "SDLI.h"
#include "ADCCtl.h"
#include "Sites.h"

namespace MFM {

  struct EWInitiator : public TimeoutAble {
    virtual void onTimeout(TimeQueue& srctq) ;
    virtual const char* getName() const { return "EWInitiator"; }
    void schedule(TimeQueue& tq) { // Schedule for now
      if (isOnTQ()) remove();
      insert(tq,0);
    }
  };

  typedef MDist<4> OurMDist;

  struct T2Tile {

    static inline T2Tile & get() {
      static T2Tile THE_INSTANCE;
      return THE_INSTANCE;
    }

    T2Tile() ;
    ~T2Tile() ;

    bool isLiving() const { return mLiving; }
    
    void setLiving(bool alive) {
      mLiving = alive;
      if (mLiving) mEWInitiator.schedule(mTimeQueue);
    }
    void setMFZId(const char * mfzid) ;
    const char * getMFZId() const ;

    Random mRandom;
    Random & getRandom() { return mRandom; }

    SDLI & getSDLI() { return mSDLI; }

    ADCCtl & getADCCtl() { return mADCCtl; }

    // HIGH LEVEL SEQUENCING
    void main() ;
    void earlyInit() ;
    void initEverything(int argc, char **argv) ;

#if 0
    void onePass() ;
#endif
    void maybeInitiateEW() ;
    void updateActiveEWs() ;
    void advanceITCs() ;
    void maybeDisplay() ;
    void shutdownEverything() ;

    bool isDone() const { return mExitRequest; }
    
    bool openITCs() ;
    void closeITCs() ;

    void processArgs() ;
    int mArgc;
    char ** mArgv;
    const char * mMFZId;
    const char * mWindowConfigPath;
    const u32 mWidth, mHeight;
    bool mExitRequest;

    void setWindowConfigPath(const char * path) ;

    T2ITC mITCs[DIR6_COUNT];
    T2EventWindow * mEWs[MAX_EWSLOT+1]; // mEWs[0] set to 0
    T2EventWindow * getEW(u32 idx) {
      if (idx <= MAX_EWSLOT) return mEWs[idx];
      return 0;
    }

    void setActive(T2EventWindow * ew) ;
    void setPassive(T2EventWindow * ew) ;

    T2EventWindow * tryAcquireEW(const UPoint at, u32 radius) ;

    void resourceAlert(ResourceType type, ResourceLevel level) ;

    void releaseEW(T2EventWindow * ew) ;

    void insertOnMasterTimeQueue(TimeoutAble & ta, u32 fromNow, s32 fuzzbits=-1) ;

    u32 now() const { return mTimeQueue.now(); }

    const Sites& getSites() const { return mSites; }
    Sites& getSites() { return mSites; }
  private:
    T2EventWindow * allocEW() ;

    void freeEW(T2EventWindow * ew) ;

    EWSet mFree;
    EWSet mActive[2];
    u32 mActiveBuffer;
    EWSet mPassive;
    TimeQueue mTimeQueue;
    void initTimeQueueDrivers() ;

    SDLI mSDLI;
    ADCCtl mADCCtl;

    //// THE SITES

    Sites mSites;

    T2EventWindow * mSiteOwners[T2TILE_WIDTH][T2TILE_HEIGHT];
    EWInitiator mEWInitiator;
    bool mLiving;
    OurMDist mMDist;

  };
}
#endif /* T2TILE_H */
