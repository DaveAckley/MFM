/* -*- C++ -*- */
#ifndef T2EVENTWINDOW_H
#define T2EVENTWINDOW_H

#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Packet.h"
#include "Rect.h"

#include <assert.h>
#include <vector>

// Spike files
#include "T2ITC.h"
#include "T2Types.h"
#include "EWSet.h"
#include "TimeoutAble.h"

namespace MFM {

  struct T2Tile; // FORWARD
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
    virtual ~T2EWStateOps() { }
  };

/**** EVENTWINDOW STATE MACHINE: EARLY STATES HACKERY ****/

#define ALL_EW_STATES_MACRO()                           \
  /*   name  custo cusrc stub desc */                   \
  XX(IDLE,     0,    0,   0,  "idle active or passive") \
  XX(AINIT,    1,    0,   0,  "initial active state")   \
  XX(AWLOCKS,  1,    1,   1,  "wait for locks")         \
  XX(ABEHAVE,  0,    0,   1,  "execute behavior")       \
  XX(ASCACHE,  1,    1,   1,  "send cache updates")     \
  XX(AWACKS,   0,    0,   1,  "wait cache upd acks")    \
  XX(ACOMMIT,  1,    1,   1,  "apply local updates")    \
  XX(PINIT,    0,    0,   1,  "initial passive state")  \
  XX(PWCACHE,  1,    1,   1,  "wait for cache updates") \
  XX(PCOMMIT,  1,    1,   1,  "apply remote updates")   \


  /*** DECLARE STATE NUMBERS **/
  typedef enum ewstatenumber {

#define XX(NAME,CUSTO,CUSRC,STUB,DESC) EWSN_##NAME,
  ALL_EW_STATES_MACRO()
#undef XX

    MAX_EW_STATE_NUMBER
  } EWStateNumber;


  /*** DECLARE PER-STATE SUBCLASSES ***/
#define YY0(FUNC) 
#define YY1(FUNC) virtual void FUNC(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) ;
#define XX(NAME,CUSTO,CUSRC,STUB,DESC)                                \
  struct T2EWStateOps_##NAME : public T2EWStateOps {                  \
    YY##CUSTO(timeout)                                                \
    YY##CUSRC(receive)                                                \
    virtual const char * getStateName() const { return #NAME; }       \
    virtual const char * getStateDescription() const { return DESC; } \
    virtual ~T2EWStateOps_##NAME() { }                                \
  };                                                                  \

  ALL_EW_STATES_MACRO()
#undef XX
#undef YY1
#undef YY0

#if 0
  enum T2EventWindowStatus {
                            UNKNOWN,
                            FREE,
                            ACTIVE,
                            PASSIVE
  };
#endif  

  struct T2EventWindow : public EWLinks, public TimeoutAble {
    // TimeoutAble methods
    virtual void onTimeout(TimeQueue& srcTq) ;

    virtual const char * getName() const ;

    virtual T2EventWindow * asEventWindow() { return this; }

    T2EventWindow(T2Tile& tile, EWSlotNum ewsn) ;

    virtual ~T2EventWindow() { }

    void schedule(TimeQueue& tq, u32 now = 0) { // Schedule for now (or then)
      if (isOnTQ()) remove();
      insert(tq,now);
    }

    T2Tile & getTile() { return mTile; }
    T2Tile & mTile;
    const EWSlotNum mSlotNum;

    EWStateNumber getEWSN() const { return mStateNum; }
    void setEWSN(EWStateNumber ewsn) {
      assert(ewsn >= 0 && ewsn < MAX_EW_STATE_NUMBER);
      mStateNum = ewsn;
    }

    /*    T2EventWindowStatus status() const { return mStatus; } */
    EWSlotNum slotNum() const { return mSlotNum; }

    void assignCenter(UPoint tileSite, u32 radius, bool activeNotPassive) {
      assert(mRadius == 0);
      assert(radius != 0);
      mCenter = tileSite;
      mRadius = radius;
      setEWSN(activeNotPassive ? EWSN_AINIT : EWSN_PINIT);
    }

    bool isAssigned() const {
      return mRadius != 0;
    }

    UPoint getCenter() const {
      assert(isAssigned());
      return mCenter;
    }

    u32 getRadius() const {
      assert(isAssigned());
      return mRadius;
    }

    void releaseCenter() {
      assert(mRadius != 0);
      mCenter = UPoint(0,0);
      mRadius = 0;
      setEWSN(EWSN_IDLE);
    }

    /*    void update() ; */
    bool tryInitiateActiveEvent(UPoint center,u32 radius) ;
    bool checkSiteAvailability() ;
    bool checkCircuitAvailability() ;
    void takeOwnershipOfRegion() ;
    bool registerWithITCs() ;

  private:
    struct CircuitInfo {
      T2ITC * mITC;
      CircuitNum mCircuitNum;
    };

    CircuitInfo mCircuits[MAX_CIRCUITS_PER_EW];

    void initializeEW() ;
    void finalizeEW() ;

    void initCircuitInfo() ;

    void addITCIfNeeded(T2ITC * itc) ;
    
    EWStateNumber mStateNum;
    /*    T2EventWindowStatus mStatus; */
    UPoint mCenter;
    u32 mRadius;
    OurT2Site mSites[EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS)];
    bool mSitesLive[EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS)];

    /*    void setStatus(T2EventWindowStatus ews) { mStatus = ews; } */
    friend class EWSet;
  };
}
#endif /* T2EVENTWINDOW_H */
