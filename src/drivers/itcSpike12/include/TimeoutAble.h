/* -*- C++ -*- */
#ifndef TIMEOUTABLE_H
#define TIMEOUTABLE_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Random.h"

//Spike files
#include "T2Constants.h"

#define time_after(a,b) ((long)((b) - (a)) < 0)
#define time_before(a,b) time_after(b,a)
#define time_after_eq(a,b) ((long)((a) - (b)) >= 0)
#define time_before_eq(a,b) time_after_eq(b,a)

#define LONG_SLEEP_MS (60*60*1000)   /* 1 hour */

namespace MFM {
  struct TimeQueue; // FORWARD

  struct TimeoutAble {

    //// TimeoutAble API METHODS:
    virtual void onTimeout(TimeQueue& srcTq) = 0;
    virtual const char * getName() const = 0;

    TimeoutAble() ;

    virtual ~TimeoutAble() ;
    void insert(TimeQueue& onTQ, u32 thisFarInFutureMs, s32 fuzzbits=-1);
    void remove() ;

    u32 getTimeout() const { return mTimeMS; }
    void printTimeout(ByteSink & bs) const ;

    bool isOnTQ() const { return mOnTQ != 0; }

    /** Reschedule this under the assumption that external
        circumstances relevant to this may have changed.  May be
        called as a courtesy by the environment.

        By default, bump() reschedules this to wake up immediately, if
        it is currently waiting on a TQ, or else does nothing.
    */
    virtual void bump() {
      if (mOnTQ !=0) {
        TimeQueue & tq = *mOnTQ;
        remove();
        insert(tq,0);
      }
    }

    void sleepOn(TimeQueue & onTQ) {
      insert(onTQ, LONG_SLEEP_MS);
    }

    u32 mTimeMS;
    u32 mNonce;
    TimeQueue * mOnTQ;

    bool operator<(const TimeoutAble & rhs) const {
      // Round 0: Shortcut identity
      if (this == &rhs) return false;

      // Round 1: Check real content
      if (time_before(mTimeMS, rhs.mTimeMS)) return true;
      if (time_after(mTimeMS, rhs.mTimeMS)) return false;

      // Round 2: Break content ties randomly
      if (mNonce < rhs.mNonce) return true;
      if (mNonce > rhs.mNonce) return false;

      // Round 3: Break random ties arbitrarily 
      return (intptr_t) this < (intptr_t) &rhs; // Reachable: Sure. Reached?
    }

  };
}
#endif /* TIMEOUTABLE_H */
