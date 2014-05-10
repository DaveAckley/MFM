/* -*- mode:C++ -*- */

/*
  MDist.h Support for Manhattan distance calculations
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
   \file MDist.h Support for Manhattan distance calculations
   \author Trent R. Small.
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef MDIST_H
#define MDIST_H

#include "itype.h"
#include "Point.h"
#include "Random.h"

namespace MFM {

  /**
   * Compute the number of sites within Manhattan distance 'radius' of a
   * given center site.  WARNING: Macro expands argument 'radius' twice!
   * Avoid side-effects!
   */

#define EVENT_WINDOW_SITES(radius) ((((radius)*2+1)*((radius)*2+1))/2+1)

  typedef enum
    {
      MANHATTAN_TABLE_RADIUS_0 = 0,
      MANHATTAN_TABLE_RADIUS_1,
      MANHATTAN_TABLE_RADIUS_2,
      MANHATTAN_TABLE_RADIUS_3,
      MANHATTAN_TABLE_RADIUS_4,
      MANHATTAN_TABLE_SHORT = MANHATTAN_TABLE_RADIUS_2,
      MANHATTAN_TABLE_LONG = MANHATTAN_TABLE_RADIUS_4,
      MANHATTAN_TABLE_EVENT = MANHATTAN_TABLE_RADIUS_4
    } TableType;

  template <u32 R>
  class MDist
  {
  public:
    static const u32 EVENT_WINDOW_DIAMETER = R*2+1;

    /**
     * Access the singleton MDist of any given size.
     */
    static MDist<R> & get();

    void FillRandomSingleDir(SPoint& pt,Random & random) const;

    u32 GetTableSize(u32 maxRadius);

    u32 ShortTableSize();

    u32 LongTableSize();

    u32 GetFirstIndex(const u32 radius) const {
      if (radius >= sizeof(m_firstIndex)/sizeof(m_firstIndex[0]))
        FAIL(ILLEGAL_ARGUMENT);
      return m_firstIndex[radius];
    }

    u32 GetLastIndex(const u32 radius) const {
      return GetFirstIndex(radius+1)-1;
    }

    const SPoint & GetPoint(const u32 index) const {
      if (index >= ARRAY_LENGTH)
        FAIL(ILLEGAL_ARGUMENT);
      return m_indexToPoint[index];
    }

    MDist();

    /**
     * Return the coding of offset as a bond if possible.  Returns -1 if
     * the given offset cannot be expressed as a max length radius bond.
     */
    s32 FromPoint(const Point<s32>& offset, u32 radius);

    /*
     * Fills pt with the point represented by bits.
     * Uses a 4-bit rep if maxRadius less than 3
     */
    void FillFromBits(SPoint& pt, u8 bits, u32 maxRadius);

  private:
    static const u32 ARRAY_LENGTH = EVENT_WINDOW_SITES(R);

    static inline u32 ManhattanArea(u32 maxDistance) {
      return EVENT_WINDOW_SITES(maxDistance);
    }

    Point<s32> m_indexToPoint[ARRAY_LENGTH];
    s32 m_pointToIndex[EVENT_WINDOW_DIAMETER][EVENT_WINDOW_DIAMETER];

    u32 m_firstIndex[R+2];  // m_firstIndex[R+1] holds 'lastIndex[R]'

  };
} /* namespace MFM */

#include "MDist.tcc"

#endif /*MDIST_H*/

