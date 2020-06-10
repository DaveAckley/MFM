/* -*- C++ -*- */
#ifndef ITCITERATOR_H
#define ITCITERATOR_H

#include "Dirs.h"
#include "Random.h"

// Spike files
#include "dirdatamacro.h"

namespace MFM {

  typedef u8 ITCDir;
  typedef ITCDir ITCDirSet[DIR6_COUNT];
  typedef s32 ITCIteratorUseCount;

  struct ITCIterator; // FORWARD
  
  struct ITCIteration {

    Random & mRandom;
    ITCIteratorUseCount mUsesRemaining;
    ITCIteratorUseCount mAverageUsesPerShuffle;
    ITCDirSet mIndices;
    u32 mIteratorsActive;

    void takeRef() {
      ++mIteratorsActive;
    }

    ITCIterator begin() ;

    void dropRef(bool used) {
      MFM_API_ASSERT_STATE(mIteratorsActive > 0);
      --mIteratorsActive;
      if (used) --mUsesRemaining; // Don't charge unless itr.next()
      if (mUsesRemaining < 0) {  // Want to shuffle
        if (mIteratorsActive == 0)
          shuffle();
        else if (-mUsesRemaining > 2*mAverageUsesPerShuffle) {
          FAIL(OUT_OF_RESOURCES);  // We'll probably regret this
        }
      }
    }

    ITCIteration(Random& random, u32 avgUsesPerShuffle)
      : mRandom(random)
      , mAverageUsesPerShuffle(avgUsesPerShuffle)
      , mIteratorsActive(0)
    {
      for (u32 i = 0; i < DIR6_COUNT; ++i)
        mIndices[i] = i;
      shuffle();
    }

    ~ITCIteration() ;

    void shuffle() {
      /* max is double avg, given uniform random */
      mUsesRemaining = mRandom.Create(mAverageUsesPerShuffle<<1) + 1;

      for (u32 i = DIR6_MAX; i > 0; --i) {
        /* generates 0..DIR6_MAX down to 0..1 */
        u32 j = mRandom.Create(i+1); 
        if (i != j) {
          ITCDir tmp = mIndices[i];
          mIndices[i] = mIndices[j];
          mIndices[j] = tmp;
        }
      }
    }

  };

  struct ITCIterator {
    ITCIteration & mIteration;
    ITCDir mNextIndex;

    ITCIterator(ITCIteration & iteration, ITCDir nextIndex=0)
      : mIteration(iteration)
      , mNextIndex(nextIndex)
    {
      mIteration.takeRef();
    }
    
    ITCIterator(const ITCIterator & itr)
      : mIteration(itr.mIteration)
      , mNextIndex(itr.mNextIndex)
    {
      mIteration.takeRef();
    }
    
    ~ITCIterator() {
      mIteration.dropRef(mNextIndex > 0);
    }

    bool hasNext() const {
      return mNextIndex < DIR6_COUNT;
    }

    ITCDir next() {
      if (!hasNext()) FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      return mIteration.mIndices[mNextIndex++];
    }
  };


}
#endif /* ITCITERATOR_H */
