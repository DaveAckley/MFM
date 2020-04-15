/* -*- C++ -*- */
#ifndef T2EVENTWINDOW_H
#define T2EVENTWINDOW_H

#include "Dirs.h"
#include "DateTimeStamp.h"

#include <assert.h>

// Spike files
#include "T2Types.h"
#include "EWSet.h"

/**** EVENTWINDOW STATE MACHINE: EARLY STATES HACKERY ****/

#define ALL_EW_STATES_MACRO()                           \
  /*   name         custo cusrc desc) */                \
  XX(IDLE,        0,    0,    "idle active or passive") \
  XX(AINIT,       1,    0,    "initial active state")   \
  XX(AWLOCKS,     1,    1,    "wait for locks")         \
  XX(ABEHAVE,     0,    0,    "execute behavior")       \
  XX(ASCACHE,     1,    1,    "send cache updates")     \
  XX(AWACKS,      0,    0,    "wait cache upd acks")    \
  XX(ACOMMIT,     1,    1,    "apply local updates")    \
  XX(PINIT,       0,    0,    "initial passive state")  \
  XX(PWCACHE,     1,    1,    "wait for cache updates") \
  XX(PCOMMIT,     1,    1,    "apply remote updates")   \

namespace MFM {

  struct T2Tile; // FORWARD

  /*** STATE NUMBERS **/
  typedef enum ewstatenumber {

#define XX(NAME,CUSTO,CUSRC,DESC) EWSN_##NAME,
                              ALL_EW_STATES_MACRO()
#undef XX

                              MAX_EW_STATE_NUMBER
  } EWStateNumber;

  enum T2EventWindowStatus {
                            UNKNOWN,
                            FREE,
                            ACTIVE,
                            PASSIVE
  };
  
  struct T2EventWindow : public EWLinks {

    virtual T2EventWindow * asEventWindow() { return this; }

    T2EventWindow(T2Tile& tile, EWSlotNum ewsn) ;

    virtual ~T2EventWindow() { }

    T2Tile & mTile;
    const EWSlotNum mSlotNum;

    EWStateNumber getEWSN() const { return mStateNum; }
    void setEWSN(EWStateNumber ewsn) {
      assert(ewsn >= 0 && ewsn < MAX_EW_STATE_NUMBER);
      mStateNum = ewsn;
    }

    T2EventWindowStatus status() const { return mStatus; }
    EWSlotNum slotNum() const { return mSlotNum; }

    void assignCenter(UPoint tileSite, u32 radius) {
      assert(mRadius == 0);
      assert(radius != 0);
      mCenter = tileSite;
      mRadius = radius;
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
    }

    void update() ;
    
  private:
    EWStateNumber mStateNum;
    T2EventWindowStatus mStatus;
    UPoint mCenter;
    u32 mRadius;
    void setStatus(T2EventWindowStatus ews) { mStatus = ews; }
    friend class EWSet;
  };
}
#endif /* T2EVENTWINDOW_H */
