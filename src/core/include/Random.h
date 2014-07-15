/*                                              -*- mode:C++ -*-
  Random.h PRNG interface for the Mersenne Twister
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Random.h PRNG interface for the Mersenne Twister
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <stdlib.h>

#include "itype.h"
#include "RandMT.h"
#include "BitVector.h"
#include "FXP.h"
#include "Fail.h"

namespace MFM
{

  /**
   * An interface for easy PRNG interaction.
   */
  class Random
  {
  public:

    /**
     * Creates a new Random instance that is ready to be used.
     */
    Random()
    { }

    /**
     * Creates a new Random instance, initialized using a specified
     * seed.
     */
    Random(u32 seed)
    {
      SetSeed(seed);
    }

    /**
     * Gets 32 well-mixed pseudo-random bits from this internal PRNG .
     *
     * @returns a 32-bit word containing pseudo-random bits.
     */
    u32 Create() ;

    /**
     * Gets a uniform pseudo-random number from 0..max-1.  FAILs
     * ILLEGAL_ARGUMENT if max==0.
     *
     * @param max The exclusive upper bound of the generated number.
     *
     * @returns A pseudo-random number in the range [0, max) .
     */
    u32 Create(u32 max) ;

    /**
     * Generates a pseudo-random boolean value.
     *
     * @returns Either \c true or \c false , determined pseudo-randomly.
     */
    bool CreateBool()
    {
      return OneIn(2);
    }

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
     * Return true pseudo-randomly, with fixed point bounds.  E.g.,
     * oddsOf(FXP16(0.6),FXP16(2)) returns true on 30% of calls.
     * FAILs ILLEGAL_ARGUMENT if outOfThisMany is <= 0.  When thisMany
     * <= 0, never returns true.  When thisMany >= outOfThisMany,
     * always returns true.
     */
    template <int P>
    bool OddsOf(FXP<P> thisMany, FXP<P> outOfThisMany)
    {
      if (outOfThisMany <= 0)
      {
	FAIL(ILLEGAL_ARGUMENT);
      }

      if (thisMany <= 0)
      {
	return false;
      }
      if (thisMany >= outOfThisMany)
      {
	return true;
      }

      return OddsOf(thisMany.intValue, outOfThisMany.intValue);
    }

    /**
     * Return a uniformly chosen pseudo-random signed number in the
     * range of min..max, with both endpoints included.  between(-1,1)
     * returns -1, 0, and 1 equally often.  FAILs with
     * ILLEGAL_ARGUMENT if max<min.
     */
    s32 Between(s32 min, s32 max) ;

    /**
     * Resets the seed for the PRNG to a given seed.
     *
     * @param seed The seed to set the internal PRNG to.
     */
    void SetSeed(u32 seed)
    {
      _generator.seedMT_MFM(seed);
    }

  private:
    RandMT _generator;

  };

  /******************************************************************************
   **                         PUBLIC INLINE FUNCTIONS                          **
   ******************************************************************************/

  inline u32 Random::Create()
  {
    return _generator.randomMT();
  }

  // Avoid modulus artifacts by sampling from round powers of 2 and rejecting
  inline u32 Random::Create(const u32 maxval)
  {
    if (maxval==0)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    int bitmask = _getNextPowerOf2(maxval)-1;
    u32 ret;
    do
    {  // loop executes less than two times on average
      ret = Create()&bitmask;
    } while (ret >= maxval);

    return ret;
  }

  inline bool Random::OddsOf(u32 thisMany, u32 outOfThisMany)
  {
    return Create(outOfThisMany) < thisMany;
  }

  inline s32 Random::Between(s32 min, s32 max)
  {
    if (max<min)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    u32 range = (u32) (max-min+1);
    return ((s32) Create(range)) + min;
  }
} /* namespace MFM */
#endif
