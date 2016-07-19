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
#include "Util.h" // For UForNumber

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
    {
      static u32 counter = 0;
      SetSeed(++counter);
    }

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
     * Gets a uniform pseudo-random number from 0..2**nbits-1.  FAILs
     * ILLEGAL_ARGUMENT if nbits > 32.
     *
     * @param nbits The number of right-justified random bits to get.
     *
     * @returns A pseudo-random number in the range [0, 2**nbits) .
     */
    u32 CreateBits(u32 nbits) ;

    /**
     * Gets a uniform pseudo-random number from 0..max-1.  FAILs
     * ILLEGAL_ARGUMENT if max==0.  Returns 0 if max==1.
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
      MFM_API_ASSERT_ARG(outOfThisMany > 0);
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
      _bitsRemaining = 0;
    }

  private:
    s32 _bitsRemaining;
    u32 _bitBuffer;
    RandMT _generator;

  };

  /******************************************************************************
   **                         PUBLIC INLINE FUNCTIONS                          **
   ******************************************************************************/

  inline u32 Random::Create()
  {
    return _generator.randomMT();
  }

  inline u32 Random::CreateBits(const u32 nbits)
  {
    if (nbits >= 32)
    {
      MFM_API_ASSERT_ARG(nbits==32);
      return Create();
    }

    // nbits <= 31
    if ((_bitsRemaining -= nbits) < 0)
    {
      _bitBuffer = Create();
      _bitsRemaining = 32-nbits;
    }
    u32 ret = _bitBuffer;
    _bitBuffer >>= nbits;
    return ret & _GetNOnes31(nbits);
  }

  // Avoid modulus artifacts by sampling from round powers of 2 and rejecting
  inline u32 Random::Create(const u32 maxval)
  {
    if (maxval<=1)
    {
      MFM_API_ASSERT_ARG(maxval==1);    // maxval==0 -> fail ILLEGAL_ARGUMENT
      return 0;
    }
    u32 nbits = _getLogBase2(maxval)+1; // +1: log2(2) == 1 -> need 2 bits
    u32 ret;
    do
    {  // loop executes less than two times on average
      ret = CreateBits(nbits);
    } while (ret >= maxval);

    return ret;
  }

  inline bool Random::OddsOf(u32 thisMany, u32 outOfThisMany)
  {
    return Create(outOfThisMany) < thisMany;
  }

  inline s32 Random::Between(s32 min, s32 max)
  {
    MFM_API_ASSERT_ARG(max>=min);
    u32 range = (u32) (max-min+1);
    return ((s32) Create(range)) + min;
  }

  template <class ITEM_TYPE, u32 SIZE>
  void Shuffle(Random & random, ITEM_TYPE array[SIZE])
  {
    for (u32 i = 0; i < SIZE; ++i)
    {
      u32 j = random.Between(i, SIZE - 1);
      ITEM_TYPE temp = array[i];
      array[i] = array[j];
      array[j] = temp;
    }
  }


  /**
   * A shuffling iterator from 0..MAX-1, to reduce
   * order-of-consideration bias.
   */
  template <u32 MAX,u32 BIT_ODDS=5>
  class RandomIterator
  {
    const u32 m_limit;
    u32 m_index;
    typedef typename UForNumber<MAX-1>::type U; // Smallest size that holds 0..MAX-1
    U m_indices[MAX];
  public:
    RandomIterator(u32 max = MAX)
      : m_limit(max)
      , m_index(0)
    {
      for (u32 i = 0; i < m_limit; ++i) m_indices[i] = (U) i;
    }

    bool ShuffleOrReset(Random & random)
    {
      bool ret = random.CreateBits(BIT_ODDS)==0;
      if (ret)
        Shuffle(random);
      else
        Reset();
      return ret;
    }

    void Shuffle(Random & random)
    {
      for (u32 i = 0; i < m_limit; ++i)
      {
        u32 j = random.Between(i, m_limit - 1);
        U temp = m_indices[i];
        m_indices[i] = m_indices[j];
        m_indices[j] = temp;
      }

      Reset();
    }

    void Reset()
    {
      m_index = 0;
    }

    u32 Next()
    {
      if (m_index >= m_limit) FAIL(OUT_OF_BOUNDS);
      return m_indices[m_index++];
    }

    bool HasNext() const
    {
      return m_index < m_limit;
    }

  };

} /* namespace MFM */
#endif
