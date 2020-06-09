/* -*- C++ -*- */
#ifndef T2EVENTWINDOW_H
#define T2EVENTWINDOW_H

#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Packet.h"
#include "Rect.h"

#include <assert.h>
#include <vector>
#include <set>

// Spike files
#include "dirdatamacro.h"
#include "T2Types.h"
#include "T2PacketBuffer.h"
#include "EWSet.h"
#include "TimeoutAble.h"

namespace MFM {

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD
  struct T2EventWindow; // FORWARD

  ////BASE CLASS OF ALL T2EW STATE MACHINE HANDLER CLASSES
  struct T2EWStateOps {
    typedef std::vector<T2EWStateOps *> T2EWStateArray;
    static T2EWStateArray mStateOpsArray;

    void resetEW(T2EventWindow *ew) ;
    virtual void timeout(T2EventWindow & ew, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual void receive(T2EventWindow & ew, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual const char * getStateName() const = 0;
    virtual const char * getStateDescription() const = 0;
    virtual bool isActive() const = 0;
    virtual ~T2EWStateOps() { }
  };

/**** EVENTWINDOW STATE MACHINE: EARLY STATES HACKERY ****/

#define ALL_EW_STATES_MACRO()                               \
  /*   name  act custo cusrc stub desc */                   \
  XX(IDLE,    1,  0,    0,   0,  "idle active or passive")  \
  XX(AINIT,   1,  1,    1,   0,  "initial active state")    \
  XX(AWLOCKS, 1,  1,    1,   0,  "wait for locks")          \
  XX(ADROP,   1,  1,    1,   0,  "finish NAKed event")      \
  XX(ABEHAVE, 1,  1,    0,   0,  "execute behavior")        \
  XX(ASCACHE, 1,  1,    1,   0,  "send cache updates")      \
  XX(AWACKS,  1,  1,    0,   0,  "wait cache upd acks")     \
  XX(ACOMMIT, 1,  1,    1,   1,  "apply local updates")     \
  XX(PINIT,   0,  0,    0,   1,  "initial passive state")   \
  XX(PWCACHE, 0,  1,    1,   1,  "wait for cache updates")  \
  XX(PCOMMIT, 0,  1,    1,   1,  "apply remote updates")    \


  /*** DECLARE STATE NUMBERS **/
  typedef enum ewstatenumber {

#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC) EWSN_##NAME,
  ALL_EW_STATES_MACRO()
#undef XX

    MAX_EW_STATE_NUMBER
  } EWStateNumber;


  /*** DECLARE PER-STATE SUBCLASSES ***/
#define YY0(FUNC) 
#define YY1(FUNC) virtual void FUNC(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) ;
#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC)                            \
  struct T2EWStateOps_##NAME : public T2EWStateOps {                  \
    YY##CUSTO(timeout)                                                \
    YY##CUSRC(receive)                                                \
    virtual const char * getStateName() const { return #NAME; }       \
    virtual const char * getStateDescription() const { return DESC; } \
    virtual bool isActive() const { return ACT != 0; }                \
    virtual ~T2EWStateOps_##NAME() { }                                \
  };                                                                  \

  ALL_EW_STATES_MACRO()
#undef XX
#undef YY1
#undef YY0

  struct CircuitInfo {
    T2ITC * mITC;
    CircuitNum mCircuitNum;
    bool mLockAcquired;
    s8 mMaxUnshippedSN;
  };

  struct T2EventWindow : public EWLinks, public TimeoutAble {
    // TimeoutAble methods
    virtual void onTimeout(TimeQueue& srcTq) ;

    virtual const char * getName() const ;

    virtual T2EventWindow * asEventWindow() { return this; }

    bool isInActiveState() const ;

    const CircuitInfo* getPassiveCircuitInfoIfAny() const ;
    const CircuitInfo& getPassiveCircuitInfo() const ;

    T2EventWindow(T2Tile& tile, EWSlotNum ewsn, const char * category) ;

    virtual ~T2EventWindow() ;

    void loadSites() ; // Tile sites -> ew sites
    void saveSites() ; // EW sites -> tile sites

    bool passiveWinsYoinkRace(const T2EventWindow & ew) const ;

    bool executeEvent() ; // This is what we're here for.  true to commit results
    typedef std::set<T2EventWindow*> EWPtrSet;
    bool resolveRacesFromPassive(EWPtrSet conflicts) ;

    void schedule(TimeQueue& tq, u32 now = 0) { // Schedule for now (or then)
      if (isOnTQ()) remove();
      insert(tq,now);
    }

    T2Tile & getTile() { return mTile; }
    T2Tile & mTile;
    const EWSlotNum mSlotNum;

    const char * getCategory() const { return mCategory; }

    EWStateNumber getEWSN() const { return mStateNum; }
    void setEWSN(EWStateNumber ewsn) ;

    /*    T2EventWindowStatus status() const { return mStatus; } */
    EWSlotNum slotNum() const { return mSlotNum; }

    void assignCenter(SPoint tileSite, u32 radius, bool activeNotPassive) ;

    bool isAssigned() const {
      return mRadius != 0;
    }

    SPoint getCenter() const {
      assert(isAssigned());
      return mCenter;
    }

    u32 getRadius() const {
      assert(isAssigned());
      return mRadius;
    }

#if 0 // UNUSED
    void releaseCenter() {
      assert(mRadius != 0);
      mCenter = SPoint(0,0);
      mRadius = 0;
      mLastSN = 0;
      setEWSN(EWSN_IDLE);
    }
#endif
    
    /*    void update() ; */
    bool tryInitiateActiveEvent(UPoint center,u32 radius) ;
    void initPassive(SPoint ctr, u32 radius, CircuitNum cn, T2ITC& itc) ;
    bool trySendLockRequests() ;
    bool checkSiteAvailability() ;
    bool checkSiteAvailabilityForPassive() ;
    bool checkCircuitAvailability() ;
    void hogEWSites() ;
    void unhogEWSites() ;
    void hogOrUnhogEWSites(T2EventWindow * ewOrNull) ;
    bool isHoggingSites() const { return mIsHoggingSites; }
    bool trySendNAK() ; // From passiveEW
    
    void registerWithITCIfNeeded(T2ITC & itc) ;
    bool isRegisteredWithAnyITCs() ;
    void handleACK(T2ITC & itc) ;
    void handleHangUp(T2ITC & itc) ;
    bool needsAnyLocks() ;
    bool hasAnyLocks() ;
    bool hasAllNeededLocks() ;
    bool trySendCacheUpdates() ;
    s32 trySendCacheUpdatePacket(CircuitInfo & ci) ;
    void applyCacheUpdatesPacket(T2PacketBuffer & pb, T2ITC & itc) ;
    bool tryReadEWAtom(ByteSource & in, u32 & sn, OurT2AtomBitVector & bv) ;
    void commitPassiveEWAndHangUp(T2ITC & itc) ;
    void commitAndReleaseActive() ;
    void dropActiveEW(bool dueToNAK) ;

    void initializeEW() ;
    void finalizeEW() ;
    void abortEW() ;

  private:
    char * mNameBuf32; //allocated so this can be const

#define UNALLOCATED_CIRCUIT_NUM 0xff
    CircuitInfo mCircuits[MAX_CIRCUITS_PER_EW];

    void initAllCircuitInfo() ;
    void initCircuitInfo(CircuitInfo & ci) ;
    void captureLockSequenceNumber(T2ITC& itc) ;
    void clearLockSequenceNumbers() ;
    u32 getLockSequenceNumber(Dir6 dir6) ;

    u32 mLockSequenceNumber[DIR6_COUNT];
    EWStateNumber mStateNum;
    /*    T2EventWindowStatus mStatus; */
    SPoint mCenter;
    u32 mRadius;
    u32 mLastSN;
    OurT2Site mSites[EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS)];
    bool mSitesLive[EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS)];
    bool mIsHoggingSites;

    const char * mCategory;

    /*    void setStatus(T2EventWindowStatus ews) { mStatus = ews; } */
    friend class EWSet;
  };

  const char * getEWStateName(EWStateNumber sn) ;

}
#endif /* T2EVENTWINDOW_H */
