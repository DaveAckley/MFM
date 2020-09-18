/* -*- C++ -*- */
#ifndef TIMEQUEUE_H
#define TIMEQUEUE_H

#include <assert.h>
#include <set>

// Core files
#include "Random.h"

// Spike files
#include "TimeoutAble.h"

namespace MFM {
  struct TimeoutAblePtrComparator {
    bool operator()(const TimeoutAble * lhs, const TimeoutAble * rhs) const {
      assert(lhs != 0 && rhs != 0);
      return (*lhs) < (*rhs);
    }
  };

  typedef std::set<TimeoutAble*,TimeoutAblePtrComparator> SetOfTimeoutAble;

  struct TimeQueue {
    TimeQueue(Random & r)
      : mRandom(r)
      , mExpiredCount(0)
    { }

    u32 getExpiredCount() const { return mExpiredCount; }
    void dumpQueue() ;
    void dumpQueue(ByteSink& bs) ;
    u32 size() const { return mPQ.size(); }
    bool isEmpty() const { return size() == 0; }
    u32 now() const ;
    TimeoutAble * getEarliestExpired() ;
    void insertRaw(TimeoutAble& ta) ;
    void removeRaw(TimeoutAble& ta) ;
    SetOfTimeoutAble mPQ;
    Random & getRandom() { return mRandom; }
    Random & mRandom;
    u32 mExpiredCount;
  };
}
#endif /* TIMEQUEUE_H */
