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
#include "Dirs.h"

namespace MFM
{

  /**
   * Compute the number of sites within Manhattan distance 'radius' of a
   * given center site.  WARNING: Macro expands argument 'radius' twice!
   * Avoid side-effects!
   */
#define EVENT_WINDOW_SITES(radius) ((((radius)*2+1)*((radius)*2+1))/2+1)

  /**
   * An enumeration of several kinds of Manhattan Distance table
   * sizes.
   */
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

  /**
   * A singleton class consisting of many utilities used for
   * calculating Manhattan Distances.
   */
  template <u32 R>
  class MDist
  {
  private:
    MDist(const MDist &) ;  // Singleton: Declare away copy ctor
    MDist & operator=(const MDist &) ; // Don't want this either

  public:
    /**
     * The diameter of an EventWindow of Radius R .
     */
    static const u32 EVENT_WINDOW_DIAMETER = R*2+1;

    /**
     * Access the singleton MDist of any given size.
     */
    static const MDist<R> & get();

    static const MDist<R> THE_INSTANCE;

    /**
     * Fills a given SPoint with a random Von Neumann unit vector.
     *
     * @param pt The SPoint to fill with a random Von Neumann unit vector.
     *
     * @param random The PRNG used to determine which unit vector to
     *               fill \c pt with.
     */
    void FillRandomSingleDir(SPoint& pt,Random & random) const;

    /**
     * Gets the area of a Manhattan Distance circle of a given radius .
     *
     * @param maxRadius The radius of the Manhattan Distance circle to
     *                  examine.
     */
    u32 GetTableSize(u32 maxRadius) const;

    /**
     * Get the lowest index corresponding to a distance of \c radius.
     * Useful for starting an event window iteration at a given distance
     *
     * \param radius The radius to find the lowest index of.
     * \sa GetLastIndex
     * \sa GetPoint
     */
    u32 GetFirstIndex(const u32 radius) const
    {
      if (radius >= sizeof(m_firstIndex)/sizeof(m_firstIndex[0]))
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      return m_firstIndex[radius];
    }

    /**
     * Get the highest index corresponding to a distance of \c radius.
     * Useful for ending an event window iteration at a given distance
     *
     * \param radius The radius to find the highest index of.
     * \sa GetFirstIndex
     * \sa GetPointIndex
     */
    u32 GetLastIndex(const u32 radius) const
    {
      return GetFirstIndex(radius+1)-1;
    }

    const SPoint & GetPoint(const u32 index) const
    {
      if (index >= ARRAY_LENGTH)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      return m_indexToPoint[index];
    }

    MDist();

    /**
     * Return the coding of offset as a bond if possible.  Returns -1 if
     * the given offset cannot be expressed as a max length radius bond.
     */
    s32 FromPoint(const Point<s32>& offset, u32 radius) const;

    /*
     * Fills pt with the point represented by bits.
     * Uses a 4-bit rep if maxRadius less than 3
     */
    void FillFromBits(SPoint& pt, u8 bits, u32 maxRadius) const;

  private:
    static const u32 ARRAY_LENGTH = EVENT_WINDOW_SITES(R);

    static inline u32 ManhattanArea(u32 maxDistance)
    {
      return EVENT_WINDOW_SITES(maxDistance);
    }

    Point<s32> m_indexToPoint[ARRAY_LENGTH];
    s32 m_pointToIndex[EVENT_WINDOW_DIAMETER][EVENT_WINDOW_DIAMETER];

    u32 m_firstIndex[R+2];  // m_firstIndex[R+1] holds 'lastIndex[R]'

    void InitEscapesByDirTable();
    u8 m_escapesByDirection[Dirs::DIR_COUNT][ARRAY_LENGTH];

    void InitHorizonsByDirTable();
    u8 m_horizonsByDirection[Dirs::DIR_COUNT][ARRAY_LENGTH];

  };

  template <u32 R>
  const MDist<R> MDist<R>::THE_INSTANCE;

} /* namespace MFM */

#include "MDist.tcc"

#endif /*MDIST_H*/
