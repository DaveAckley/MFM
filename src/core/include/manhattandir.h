/*                                              -*- mode:C++ -*-
  manhattandir.h Support for mapping manhattan distances in and out of 1D form
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
  \file manhattandir.h Support for mapping manhattan distances in and out of 1D form
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef MANHATTANDIR_H
#define MANHATTANDIR_H

#include "itype.h"
#include "point.h"

namespace MFM {

/**
 * Compute the number of sites within Manhattan distance 'radius' of a
 * given center site.  WARNING: Macro expands argument 'radius' twice!
 * Avoid side-effects!
 */

#define EVENT_WINDOW_SITES(radius) ((((radius)*2+1)*((radius)*2+1))/2+1)

// Let's let the table type and the max bond length be the same thing.
// Can simplify it later.

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
class ManhattanDir
{
public:
  static const u32 EVENT_WINDOW_DIAMETER = R*2+1;

  static Point<s32>& FlipAxis(Point<s32>& pt, bool xAxis);

  /**
   * Access the singleton ManhattanDir of any given size.
   */
  static ManhattanDir<R> & get();

  void FillRandomSingleDir(SPoint& pt);

  u32 GetTableSize(TableType type);

  u32 ShortTableSize();

  u32 LongTableSize();
  
  ManhattanDir();

  /**
   * Return the coding of offset as a bond if possible.  Returns -1 if
   * the given offset cannot be expressed as a bond of the given type
   * (or radius, same thing).
   */
  s32 FromPoint(const Point<s32>& offset, TableType type);

  /* 
   * Fills pt with the point represented by bits.
   * If this is a short bond (i.e. a 4-bit rep),
   * sbond needs to be true.
   */
  void FillFromBits(SPoint& pt, u8 bits,
                    TableType type);

  Point<s32>& GetSEWindowPoint(u32 index)
  { return m_southeastSubWindow[index]; }

  SPoint* GetSESubWindow()
  { return m_southeastSubWindow; }


private:
  static const u32 ARRAY_LENGTH = EVENT_WINDOW_SITES(R);

  static inline u32 ManhattanArea(u32 maxDistance) {
    return EVENT_WINDOW_SITES(maxDistance);
  }

  /* This only works if R is a power of two! */
  Point<s32> m_southeastSubWindow[R];

  Point<s32> m_indexToPoint[ARRAY_LENGTH];
  s32 m_pointToIndex[EVENT_WINDOW_DIAMETER][EVENT_WINDOW_DIAMETER];

  u32 m_firstIndex[R+2];  // m_firstIndex[R+1] holds 'lastIndex[R]'

  u32 GetBondSize(TableType type);
  
};
} /* namespace MFM */

#include "manhattandir.tcc"

#endif /*MANHATTANDIR_H*/

