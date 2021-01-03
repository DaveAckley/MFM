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
#include "Circuit.h"

namespace MFM {

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD
  struct T2EventWindow; // FORWARD
  struct T2ActiveEventWindow; // FORWARD
  struct T2PassiveEventWindow; // FORWARD

  ////BASE CLASS OF ALL T2EW STATE MACHINE HANDLER CLASSES
  struct T2EWStateOps {
    typedef std::vector<T2EWStateOps *> T2EWStateArray;
    static T2EWStateArray mStateOpsArray;

    void resetEW(T2EventWindow *ew) ;
    virtual void timeout(T2ActiveEventWindow & ew, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual void receive(T2ActiveEventWindow & ew, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual void timeout(T2PassiveEventWindow & ew, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual void receive(T2PassiveEventWindow & ew, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual const char * getStateName() const = 0;
    virtual const char * getStateDescription() const = 0;
    virtual bool isActive() const = 0;
    virtual ~T2EWStateOps() { }
  };

/**** EVENTWINDOW STATE MACHINE: EARLY STATES HACKERY ****/

#define ALL_EW_STATES_MACRO()                               \
  /*   name  act custo cusrc stub desc */                   \
  XX(IDLE,    1,  0,    0,   0,  "idle active or passive")  \
  XX(AINIT,   1,  1,    0,   0,  "initial active state")    \
  XX(AWLOCKS, 1,  1,    0,   0,  "wait for locks")          \
  XX(ADROP,   1,  1,    0,   0,  "finish NAKed event")      \
  XX(ABEHAVE, 1,  1,    0,   0,  "execute behavior")        \
  XX(ASCACHE, 1,  1,    0,   0,  "send cache updates")      \
  XX(AWACKS,  1,  1,    0,   0,  "wait cache upd acks")     \
  XX(ACOMMIT, 1,  1,    0,   1,  "apply local updates")     \
  XX(PINIT,   0,  0,    0,   1,  "initial passive state")   \
  XX(PRESOLVE,0,  0,    0,   1,  "handling a RING")         \
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
  /*Doing YY CUS ACT expansion:
   YY00 default passive, YY01 default active
   YY10 custom  passive, YY11 custom active
  */

#define ZZ0() T2PassiveEventWindow
#define ZZ1() T2ActiveEventWindow
#define YY0(FUNC,ACT) 
#define YY1(FUNC,ACT) virtual void FUNC(ZZ##ACT() & ew, T2PacketBuffer & pb, TimeQueue& tq) ;
#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC)                            \
  struct T2EWStateOps_##NAME : public T2EWStateOps {                  \
    YY##CUSTO(timeout,ACT)                                            \
    YY##CUSRC(receive,ACT)                                            \
    virtual const char * getStateName() const { return #NAME; }       \
    virtual const char * getStateDescription() const { return DESC; } \
    virtual bool isActive() const { return ACT != 0; }                \
    virtual ~T2EWStateOps_##NAME() { }                                \
  };                                                                  \

  ALL_EW_STATES_MACRO()
#undef XX
#undef YY1
#undef YY0
#undef ZZ1
#undef ZZ0

  struct T2EventWindow : public EWLinks, public TimeoutAble {
    virtual const T2ActiveEventWindow * asActiveEW() const { return 0; }
    virtual const T2PassiveEventWindow * asPassiveEW() const { return 0; }

    virtual void unbindCircuitsAsNeeded() = 0;

    virtual T2ActiveEventWindow * asActiveEW() {
      return const_cast<T2ActiveEventWindow*>(asActiveEW());
    }
    virtual T2PassiveEventWindow * asPassiveEW() { 
      return const_cast<T2PassiveEventWindow*>(asPassiveEW());
    }
    bool isActiveEW() const { return asActiveEW() != 0; }
    bool isPassiveEW() const { return asPassiveEW() != 0; }

    // TimeoutAble methods
    virtual void onTimeout(TimeQueue& srcTq) ;

    virtual const char * getName() const ;

    virtual T2EventWindow * asEventWindow() { return this; }

    T2EventWindow(T2Tile& tile, EWSlotNum ewsn, const char * category) ;

    virtual ~T2EventWindow() ;

    void loadSites() ; // Tile sites -> ew sites
    void saveSites() ; // EW sites -> tile sites

    bool passiveWinsYoinkRace(const T2EventWindow & ew) const ;

    typedef std::set<T2EventWindow*> EWPtrSet;

    void schedule(TimeQueue& tq, u32 now = 0) { // Schedule for now (or then)
      if (isOnTQ()) remove();
      insert(tq,now);
    }

    T2Tile & getTile() { return mTile; }
    T2Tile & mTile;
    const EWSlotNum mSlotNum;
    EWSlotNum getSlotNum() const { return mSlotNum; }

    const char * getCategory() const { return mCategory; }

    EWStateNumber getEWSN() const { return mStateNum; }
    void setEWSN(EWStateNumber ewsn) ;

    /*    T2EventWindowStatus status() const { return mStatus; } */
    /*use getSlotNum doh    EWSlotNum slotNum() const { return mSlotNum; } */

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


    void hogEWSites() ;
    void unhogEWSites() ;
    void hogOrUnhogEWSites(T2EventWindow * ewOrNull) ;
    bool isHoggingSites() const { return mIsHoggingSites; }
    
    bool tryReadEWAtom(ByteSource & in, u32 & sn, OurT2AtomBitVector & bv) ;

    void initializeEW() ;
    void finalizeEW() ;
    void abortEW() ;

  protected:
    char * mNameBuf32; //allocated so getname() can be const
    void _setEWSNRaw(EWStateNumber ewsn) {
      mStateNum = ewsn;
    }
#if 0
    void freeAllCircuits() ;
#endif
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

  struct T2ActiveEventWindow : public T2EventWindow {
    virtual const T2ActiveEventWindow * asActiveEW() const { return this; }
    virtual T2ActiveEventWindow * asActiveEW() { return this; }
    virtual void unbindCircuitsAsNeeded() ;
    virtual const char * getName() const ;

    T2ActiveEventWindow(T2Tile& tile, EWSlotNum ewsn, const char * category) ;

    const Circuit* getActiveCircuitIfAny(T2ITC& toITC) const ;

    bool tryInitiateActiveEvent(UPoint center,u32 radius) ;

    bool executeEvent() ; // This is what we're here for.  true to commit results, false to kill center

    bool doBehavior() ; // Actually run the center atom's event transition code

    bool checkSiteAvailabilityForActive() ;

    void commitAndReleaseActive() ;
    void justKillCenterAndCommitThat() ;

    void dropActiveEW(bool dueToNAK) ;

    void handleAnswer(T2ITC & itc) ;
    void handleHangUp(T2ITC & itc) ;
    void handleBusy(T2ITC & itc) ;
    bool hasAllNeededLocks() ;
    bool needsAnyLocks() ;
    bool hasAnyLocks() ;
    void registerWithITCIfNeeded(T2ITC & itc) ;

    void sendDropsExceptTo(T2ITC * notThisOne) ;
    bool trySendCacheUpdates() ;
    s32 trySendCacheUpdatePacket(Circuit & ci) ;
    bool trySendLockRequests() ;

    bool trySendDropVia(T2ITC & itc) ;

    const Circuit * getActiveCircuitForITCIfAny(const T2ITC & itc) const ;

    Circuit mActiveCircuits[CIRCUITS_PER_ACTIVE_EW];
    u64 mActiveEventCountForAge;

  private: ////DEMO PHYSICS
    void doSplitsAtTheEndOfTheUniverse(OurT2Atom &atom) ;
    void doDRegAndRes(OurT2Atom &atom) ;
  };

  struct T2PassiveEventWindow : public T2EventWindow {
    virtual const T2PassiveEventWindow * asPassiveEW() const { return this; }
    virtual T2PassiveEventWindow * asPassiveEW() { return this; }
    virtual void unbindCircuitsAsNeeded() ;
    virtual const char * getName() const ;

    T2PassiveEventWindow(T2Tile& tile,
                         EWSlotNum ewsn,
                         const char * category,
                         T2ITC & itc) ;

    void initPassive(SPoint ctr, u32 radius, bool yoink) ;
    bool checkSiteAvailabilityForPassive() ;
    void commitPassiveEWAndHangUp() ;
    void applyCacheUpdatesPacket(T2PacketBuffer & pb, T2ITC & itc) ;

    void handleDrop(T2ITC& itc) ;

    const Circuit& getPassiveCircuit() const { return mPassiveCircuit; }
    Circuit& getPassiveCircuit() { return mPassiveCircuit; }

    Circuit mPassiveCircuit;

    bool resolveRacesFromPassive(EWPtrSet conflicts) ;

    bool passiveWinsYoinkRace(const T2ActiveEventWindow & ew) const ;

    bool trySendBusy() ;

    void resetPassiveEW() ;
  };

  const char * getEWStateName(EWStateNumber sn) ;

}
#endif /* T2EVENTWINDOW_H */
