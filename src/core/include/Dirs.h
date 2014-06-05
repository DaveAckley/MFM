/*                                              -*- mode:C++ -*-
  Dirs.h Euclidean direction system
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
  \file Dirs.h Euclidean direction system
  \author Trent R. Small.
  \author David H. Ackley
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef DIRS_H
#define DIRS_H

#include "Point.h"

namespace MFM {

  typedef u32 Dir;

  class Dirs
  {
  public:
    static const Dir NORTH     = 0;
    static const Dir NORTHEAST = 1;
    static const Dir EAST      = 2;
    static const Dir SOUTHEAST = 3;
    static const Dir SOUTH     = 4;
    static const Dir SOUTHWEST = 5;
    static const Dir WEST      = 6;
    static const Dir NORTHWEST = 7;
    static const Dir DIR_COUNT = 8;

    /**
     * true iff dir is a corner direction.
     */
    static bool IsCorner(Dir dir) { return dir&1; }

    /**
     * true iff dir is a face direction.
     */
    static bool IsFace(Dir dir) { return !IsCorner(dir); }

    /**
     * The next dir clockwise from dir.  Note that for all dir,
     * IsCorner(dir)==IsFace(CWDir(dir)), and dir==CWDir(CCWDir(dir))
     */
    static Dir CWDir(Dir dir) { return (dir+1)%DIR_COUNT; }

    /**
     * Gets the direction opposite the one specified.
     *
     * @returns The direction opposite the one specified.
     */
    static Dir OppositeDir(Dir dir)
    { return (dir + (DIR_COUNT / 2)) % DIR_COUNT; }

    /**
     * The next dir counter-clockwise from dir.  Note that for all dir,
     * IsCorner(dir)==IsFace(CCWDir(dir)), and dir==CCWDir(CWDir(dir))
     */
    static Dir CCWDir(Dir dir) { return (dir+DIR_COUNT-1)%DIR_COUNT; }

    /**
     * Adds a Dir to a Dir mask.
     *
     * @param mask The mask to add a Dir to.
     *
     * @param dir The Dir to add to mask.
     *
     * @returns mask with dir added to it.
     */
    static inline u32 AddDirToMask(u32 mask, Dir dir)
    { return mask | (1 << dir); }

    /**
     * Removes a Dir from a Dir mask.
     *
     * @param mask The Dir mask to remove a Dir from.
     *
     * @param dir The Dir to remove from mask.
     *
     * @returns mask with dir removed from it.
     */
    static inline u32 RemoveDirInMask(u32 mask, Dir dir)
    { return mask & (~(1 << dir)); }

    /**
     * Checks a Dir mask to see if a Dir is inside it.
     *
     * @param mask The mask to check Dir membership of.
     *
     * @param dir The Dir to check mask for membership of.
     *
     * @return true if dir is inside mask, else false.
     */
    static inline bool TestDirInMask(u32 mask, Dir dir)
    { return mask & (1 << dir); }

    static void FillDir(SPoint& pt, Dir dir);

    static Dir FromOffset(SPoint& pt);

    static inline SPoint & FlipXAxis(SPoint & pt) {
      pt.SetX(-pt.GetX());
      return pt;
    }

    static inline SPoint & FlipYAxis(SPoint & pt) {
      pt.SetY(-pt.GetY());
      return pt;
    }

    static SPoint FlipSEPointToCorner(const Point<s32>& pt, const Dir corner);

  };

} /* namespace MFM */

#endif /*DIRS_H*/
