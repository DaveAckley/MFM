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
#include "T2Main.h"
#include "T2Types.h"
#include "T2ITC.h"
#include "EWSet.h"
#include "SDLI.h"
#include "ADCCtl.h"
#include "Sites.h"
#include "Trace.h"
#include "CPUFreq.h"
#include "T2TileStats.h"
#include "T2FlashTrafficManager.h"
#include "T2UIComponents.h"
#include "TraceLogInfo.h" /*for TraceLogInfo, TraceLogDirManager */

namespace MFM {

  struct EWInitiator : public TimeoutAble {
    EWInitiator() ;
    virtual void onTimeout(TimeQueue& srctq) ;
    virtual const char* getName() const { return "EWInitiator"; }
  };

  struct CoreTempChecker : public TimeoutAble {
    CoreTempChecker() ;
    virtual void onTimeout(TimeQueue& srctq) ;
    virtual const char* getName() const { return "CoreTempChk"; }
  };

  struct KITCPoller : public TimeoutAble {
    virtual void onTimeout(TimeQueue& srctq) ;
    virtual const char* getName() const { return "KITCPoller"; }
    KITCPoller(T2Tile& tile) ;
    static u32 getKITCEnabledStatusFromStatus(u32 status, Dir8 dir8) {
      return (status>>(dir8<<2))&0xf;
    }
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

  typedef std::map<u32,u32> AtomTypeCountMap;
  
  struct T2Tile : public T2Main {

    // Get the T2Main singleton set up;
    static void initInstance() { new T2Tile(); }

    static T2Tile & get() {
      T2Main * o = &T2Main::get();
      T2Tile * t = dynamic_cast<T2Tile*>(o);
      MFM_API_ASSERT_NONNULL(t);
      return (T2Tile&) *t;
    }

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

    // GENERAL SERVICE METHODS
    void debugSetup() ; // Whatever we're currently working on
    void seedPhysics(u32 type) ;
    void clearPrivateSites() ;
    void traceSite(const UPoint at, const char * msg = "", Logger::Level level = Logger::DBG) const ;

    ///
    void setMFZId(const char * mfzid) ;
    const char * getMFZId() const ;

    SDLI & getSDLI() { return mSDLI; }

    ADCCtl & getADCCtl() { return mADCCtl; }

    T2FlashTrafficManager & getFlashTrafficManager() { return mFlashTrafficManager; }

    void addRandomSyncTag(ByteSink & bs) ;
    bool tryReadRandomSyncTag(ByteSource& bs, s32 & got) ;

    bool isTracingActive() const {
      return mTraceLoggerPtr != 0;
    }

    bool tlog(const Trace & tb) ;

    TraceLogDirManager & getTraceLogDirManager() { return mTraceLogDirManager; }
    
    // Draw a kill screen before dying.
    void showFail(const char * file, int line, const char * msg) ;

    static s32 makeTag() ;
    void dumpTrace(const char * path = 0) ;
    void dumpTrace(const u32 tag, u8 range, BPoint offset) ;
    void startTracing(const char * path, s32 synctag = makeTag()) ;
    void stopTracing(s32 synctag = -makeTag()) ;
    void traceEventStats() ;

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
    void requestExit() { mExitRequest = true; }
    
    void resetITCs() ;
    void closeITCs() ;

    void processArgs() ;

    void setWindowConfigPath(const char * path) ;

    T2ActiveEventWindow & getActiveEW(u32 idx) {
      MFM_API_ASSERT_ARG(idx<MAX_EWSLOT);
      return *mEWs[idx];
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

    void releaseActiveEW(T2EventWindow & ew, bool countInStats) ;

#if 0 // UNUSED
    void releaseEW(T2EventWindow * ew) ;
#endif

    void insertOnMasterTimeQueue(TimeoutAble & ta, u32 fromNow, s32 fuzzbits=-1) ;


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

    const Rect & getOwnedRect() const {
      return mOwnedRect;
    }

    const Rect & getVisibleRect(Dir6 dir6) const {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCVisible[dir6];
    }

    const Rect & getNeighborOwnedRect(Dir6 dir6) const {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCNeighborOwned[dir6];
    }

    const Rect & getCacheRect(Dir6 dir6) const {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCCache[dir6];
    }
    const Rect & getVisibleAndCacheRect(Dir6 dir6) const {
      MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
      return mITCVisibleAndCache[dir6];
    }

    KITCPoller & getKITCPoller() { return mKITCPoller; }

    const char * getWindowConfigPath() { return mWindowConfigPath; }

    CPUFreq & getCPUFreq() { return mCPUFreq; }

    
  private:
    TraceLogger* mTraceLoggerPtr;
    TraceLogDirManager mTraceLogDirManager;

    int mArgc;
    char ** mArgv;
    const char * mMFZId;
    const char * mWindowConfigPath;
    const u32 mWidth, mHeight;
    bool mExitRequest;

    T2ITC mITCs[DIR6_COUNT];

  public:  // Hey it's const
    const Rect mOwnedRect;
    const Rect mITCVisible[DIR6_COUNT];
    const Rect mITCCache[DIR6_COUNT];
    const Rect mITCVisibleAndCache[DIR6_COUNT];
    const Rect mITCNeighborOwned[DIR6_COUNT];

    void freeEW(T2EventWindow & ew) ; //Public for T2EventWindow to call on abort?

#if 0
    void resetTotalStats() ;

    u64 getTotalActiveEventsInitiated() const {
      return  mTotalActiveEventsInitiated;
    }
    u64 getTotalActiveEmptyEventsConsidered() const {
      return  mTotalActiveEmptyEventsConsidered;
    }
    u64 getTotalActiveEventsPerformed() const {
      return  mTotalActiveEventsPerformed;
    }
    u64 getTotalNonemptyActiveEventsPerformed() const {
      return  mTotalNonemptyActiveEventsPerformed;
    }
    u64 getTotalActiveEventsCompleted() const {
      return  mTotalActiveEventsCompleted;
    }
    u64 getTotalNonemptyActiveEventsCompleted() const {
      return  mTotalNonemptyActiveEventsCompleted;
    }

    void incrTotalActiveEventsInitiated() {
      ++mTotalActiveEventsInitiated;
    }
    void incrTotalActiveEmptyEventsConsidered() {
      ++mTotalActiveEmptyEventsConsidered;
    }
    void incrTotalActiveEventsPerformed() {
      ++mTotalActiveEventsPerformed;
    }
    void incrTotalNonemptyActiveEventsPerformed() {
      ++mTotalNonemptyActiveEventsPerformed;
    }
    void incrTotalActiveEventsCompleted() {
      ++mTotalActiveEventsCompleted;
    }
    void incrTotalNonemptyActiveEventsCompleted() {
      ++mTotalNonemptyActiveEventsCompleted;
    }
#endif

    const T2TileStats & getStats() const { return mT2TileStats; }
    T2TileStats & getStats() { return mT2TileStats; }

  private:
    T2ActiveEventWindow * mEWs[MAX_EWSLOT];

    u32 considerSiteForEW(UPoint idx) ;
    u32 getRadius(const OurT2Atom & atom) ; // STUB UNTIL UCR EXISTS
    void recordCompletedEvent(OurT2Site & site) ;

    const char * coordMap(u32 x, u32 y) const ;
    void dumpTileMap(ByteSink& bs, bool csv) const ;
    void dumpITCRects(ByteSink& bs) const ;

    T2ActiveEventWindow * allocEW() ;

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
    T2TileStats mT2TileStats;

    //// HW CONTROL & MISC
    CPUFreq mCPUFreq;
    CoreTempChecker mCoreTempChecker;    

    //// Active Radio Groups
    MFMRunRadioGroup mMFMRunRadioGroup;
    
    //// FLASH TRAFFIC MANAGEMENT
    T2FlashTrafficManager mFlashTrafficManager;

    void initRollingTraceDir(u32 targetMB) ;
    void rollTracing() ;

    OString128 mRollingTraceDir;
    u32 mRollingTraceTargetKB;
    u32 mRollingTraceSpinner;

  protected:
    T2Tile() ;
    ~T2Tile() ;
  };
}
#endif /* T2TILE_H */
