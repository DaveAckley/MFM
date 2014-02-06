#ifndef _RANDOM_H_               /* -*- mode: C++ -*- */
#define _RANDOM_H_

#include <stdlib.h>

#include "itype.h"
#include "randmt.h"
#include "bitops.h"
#include "fail.h"

namespace MFM
{

  class Random {
  public:
    Random() { }

    Random(u32 seed) { SetSeed(seed); }

    /**
     * Get 32 well-mixed pseudo-random bits
     */ 
    u32 Create() ;

    /**
     * Get a uniform pseudo-random number from 0..max-1.  FAILs
     * ILLEGAL_ARGUMENT if max==0.
     */
    u32 Create(u32 max) ;

    /**
     * Get a pseudo-random truth value
     */ 
    bool CreateBool() { return OneIn(2); }

    /**
     * Return true pseudo-randomly. with a chance of 1-in-odds.  E.g.,
     * oneIn(10) returns true on 10% of calls.  When odds == 1, always
     * returns true.  FAILs ILLEGAL_ARGUMENT if odds is 0.
     */

    bool OneIn(u32 odds) { return OddsOf(1,odds); }

    /**
     * Return true pseudo-randomly. with thisMany times outOfThisMany
     * times.  E.g., oddsOf(1,10) returns true on 10% of calls.  When
     * thisMany == 0, never returns true.  When thisMany >=
     * outOfThisMany, always returns true.  FAILs ILLEGAL_ARGUMENT if
     * outOfThisMany is 0.
     */

    bool OddsOf(u32 thisMany, u32 outOfThisMany) ;

    /**
     * Return a uniformly chosen pseudo-random signed number in the
     * range of min..max, with both endpoints included.  between(-1,1)
     * returns -1, 0, and 1 equally often.  FAILs with
     * ILLEGAL_ARGUMENT if max<min.
     */
    s32 Between(s32 min, s32 max) ;

    void SetSeed(u32 seed) { _generator.seedMT_MFM(seed); }

  private:
    RandMT _generator;
    
  };

  /******************************************************************************
   **                         PUBLIC INLINE FUNCTIONS                          **
   ******************************************************************************/

  inline u32 Random::Create() {
    return _generator.randomMT();
  }

  // Avoid modulus artifacts by sampling from round powers of 2 and rejecting
  inline u32 Random::Create(const u32 maxval) {
    if (maxval==0)
      FAIL(ILLEGAL_ARGUMENT);
    int bitmask = _getNextPowerOf2(maxval)-1;
    u32 ret;
    do {  // loop executes less than two times on average
      ret = Create()&bitmask;
    } while (ret >= maxval);
    return ret;
  }

  inline bool Random::OddsOf(u32 thisMany, u32 outOfThisMany) {
    return Create(outOfThisMany) < thisMany;
  }

  inline s32 Random::Between(s32 min, s32 max) {
    if (max<min)
      FAIL(ILLEGAL_ARGUMENT);
    u32 range = (u32) (max-min+1);
    return ((s32) Create(range)) + min;
  }

} /* namespace MFM */
#endif
