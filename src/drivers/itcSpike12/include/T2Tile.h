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
    EWInitiator() ;
    virtual void onTimeout(TimeQueue& srctq) ;
    virtual const char* getName() const { return "EWInitiator"; }
  };

  struct KITCPoller : public TimeoutAble {
    virtual void onTimeout(TimeQueue& srctq) ;
    virtual const char* getName() const { return "KITCPoller"; }
    KITCPoller(T2Tile& tile) ;
    static u32 getKITCEnabledStatusFromStatus(u32 status, Dir8 dir8) { return (status>>(dir8<<2))&0xf; }
    static u32 updateKITCEnabledStatusFromStatus(u32 status, Dir8 dir8, u32 val) ;
    u32 getKITCEnabledStatus(Dir8 dir8) { return getKITCEnabledStatusFromStatus(mKITCEnabledStatus, dir8); }
    void setKITCEnabledStatus(Dir8 dir8, u32 val) {
      mKITCEnabledStatus = updateKITCEnabledStatusFromStatus(mKITCEnabledStatus, dir8, val);
    }
    T2Tile & mTile;
    ITCIteration mITCIteration;
    s32 mKITCStatusFD;
    u32 mKITCEnabledStatus;
  };

  typedef MDist<4> OurMDist;

  struct T2Tile {

    static inline T2Tile & get() {
      static bool underConstruction;
      if (underConstruction) {
        // Go bare lib: LOG etc may not be available yet
        fprintf(stderr,"%s Reentry during construction\n",__PRETTY_FUNCTION__);
        FAIL(ILLEGAL_STATE);
      }
      underConstruction = true;
      {
        static T2Tile THE_INSTANCE;
        underConstruction = false;
        return THE_INSTANCE;
      }
    }

    T2Tile() ;
    ~T2Tile() ;

    bool isLiving() const { return mLiving; }
    
    void setLiving(bool alive) {
      mLiving = alive;
      if (mLiving) mEWInitiator.schedule(mTimeQueue);
      else if (mEWInitiator.isOnTQ()) mEWInitiator.remove();
    }

    bool isListening() const { return mListening; }
    
    void setListening(bool listen) {
      mListening = listen;
      if (mListening) mPacketPoller.schedule(mTimeQueue);
      else if (mPacketPoller.isOnTQ()) mPacketPoller.remove();
    }

    void setMFZId(const char * mfzid) ;
    const char * getMFZId() const ;

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
    bool maybeInitiateEW() ;
#if 0
    void updateActiveEWs() ;
#endif
    void advanceITCs() ;
    void maybeDisplay() ;
    void shutdownEverything() ;

    bool isDone() const { return mExitRequest; }
    
    void resetITCs() ;
    void closeITCs() ;

    void processArgs() ;

    void setWindowConfigPath(const char * path) ;

    T2EventWindow * getEW(u32 idx) {
      if (idx <= MAX_EWSLOT) return mEWs[idx];
      return 0;
    }

    T2ITC & getITC(u32 dir6) {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCs[dir6];
    }

    /* XXX UNIMPLEMENTED? UNUSED? 
    void setActive(T2EventWindow * ew) ;
    void setPassive(T2EventWindow * ew) ;
    */
    bool tryAcquireEW(const UPoint at, u32 radius, bool forActive) ;

    void resourceAlert(ResourceType type, ResourceLevel level) ;

    void releaseEW(T2EventWindow * ew) ;

    void insertOnMasterTimeQueue(TimeoutAble & ta, u32 fromNow, s32 fuzzbits=-1) ;

    u32 now() const { return mTimeQueue.now(); }
    TimeQueue & getTQ() { return mTimeQueue; }

    const Sites& getSites() const { return mSites; }
    Sites& getSites() { return mSites; }
    OurMDist & getMDist() { return mMDist; }
    T2EventWindow * getSiteOwner(UPoint idx) {
      MFM_API_ASSERT_ARG(idx.GetX() < T2TILE_WIDTH &&
                         idx.GetY() < T2TILE_HEIGHT);
      return mSiteOwners[idx.GetX()][idx.GetY()];
    }

    void setSiteOwner(UPoint idx,T2EventWindow * owner) {
      // Stay in bounds
      MFM_API_ASSERT_ARG(idx.GetX() < T2TILE_WIDTH &&
                         idx.GetY() < T2TILE_HEIGHT);
      // Only set when clear or clear when set
      MFM_API_ASSERT_ARG((mSiteOwners[idx.GetX()][idx.GetY()] == 0) !=
                         (owner == 0));
      mSiteOwners[idx.GetX()][idx.GetY()] = owner;
    }

    const Rect & getVisibleRect(Dir6 dir6) {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCVisible[dir6];
    }
    const Rect & getCacheRect(Dir6 dir6) {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCCache[dir6];
    }
    const Rect & getVisibleAndCacheRect(Dir6 dir6) {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCVisibleAndCache[dir6];
    }

    KITCPoller & getKITCPoller() { return mKITCPoller; }

    const char * getWindowConfigPath() { return mWindowConfigPath; }

  private:
    Random mRandom;
    TimeQueue mTimeQueue;

    int mArgc;
    char ** mArgv;
    const char * mMFZId;
    const char * mWindowConfigPath;
    const u32 mWidth, mHeight;
    bool mExitRequest;

    T2ITC mITCs[DIR6_COUNT];

  
  public:  // Hey it's const
    const Rect mITCVisible[DIR6_COUNT];
    const Rect mITCCache[DIR6_COUNT];
    const Rect mITCVisibleAndCache[DIR6_COUNT];

    void freeEW(T2EventWindow * ew) ; //Public for T2EventWindow to call on abort?

  private:
    T2EventWindow * mEWs[MAX_EWSLOT+1]; // mEWs[0] set to 0

    u32 considerSiteForEW(UPoint idx) ;
    u32 getRadius(const OurT2Atom & atom) ; // STUB UNTIL UCR EXISTS
    void recordCompletedEvent(OurT2Site & site) ;

    T2EventWindow * allocEW() ;

    EWSet mFree;
    void initTimeQueueDrivers() ;

    SDLI mSDLI;
    ADCCtl mADCCtl;

    //// THE SITES

    Sites mSites;

    T2EventWindow * mSiteOwners[T2TILE_WIDTH][T2TILE_HEIGHT];
    EWInitiator mEWInitiator;
    KITCPoller mKITCPoller;
    bool mLiving;
    T2ITCPacketPoller mPacketPoller;
    bool mListening;
    OurMDist mMDist;

    s32 mKITCEnabledFD;

    //// STATS
    u64 mTotalEventsCompleted;

  };
}
#endif /* T2TILE_H */
