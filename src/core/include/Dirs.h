/*                                              -*- mode:C++ -*-
  Dirs.h Euclidean direction system
  Copyright (C) 2014, 2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2017 Ackleyshack, LLC.   All rights reserved.

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
  \author Elena S. Ackley
  \date (C) 2014,2017 All rights reserved.
  \lgpl
 */
#ifndef DIRS_H
#define DIRS_H

#include "Point.h"
#include "Fail.h"

namespace MFM
{

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
       Check if a value is a legal Dir
     */
    static bool IsLegalDir(s32 val)
    {
      return val >= (s32) NORTH && val <= (s32) NORTHWEST;
    }

    /**
       check if a dir is valid for layout
    */
    static bool IsValidDir(Dir dir, bool isStaggered)
    {
      if(!IsLegalDir((s32) dir))
	return false;
      return !isStaggered || ((dir != NORTH) && (dir != SOUTH));
    }

    /**
     * map a dir to a readable string
     */
    static const char * GetName(Dir dir)
    {
      switch (dir)
      {
      case NORTH: return "North";
      case EAST: return "East";
      case SOUTH: return "South";
      case WEST: return "West";
      case NORTHEAST: return "Northeast";
      case SOUTHEAST: return "Southeast";
      case SOUTHWEST: return "Southwest";
      case NORTHWEST: return "Northwest";
      case (Dir) -1: return "No direction";
      default: return "INVALID DIRECTION";
      }
    }

    /**
     * map a dir to a 2 character code
     */
    static const char * GetCode(Dir dir)
    {
      switch (dir)
      {
      case NORTH: return "NT";
      case EAST: return "ET";
      case SOUTH: return "ST";
      case WEST: return "WT";
      case NORTHEAST: return "NE";
      case SOUTHEAST: return "SE";
      case SOUTHWEST: return "SW";
      case NORTHWEST: return "NW";
      case (Dir) -1: return "--";
      default: return "XX";
      }
    }

    /**
     * true iff dir is a corner direction.
     */
    static bool IsCorner(Dir dir) { return dir&1; }

    /**
     * The next dir clockwise from dir.
     * Note that in CHECKERBOARD layout, for all dir,
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
     * The next dir counter-clockwise from dir.
     * Note In CHECKERBOARD layout, for all dir,
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

    /**
     * Given a Dir, will fill a SPoint with double unit offsets representing
     * this Dir for site coordinates (not grid rows and columns).
     * Multiplied by two, for subsequent multiply by (OWNED_WIDTH/2, OWNED_HEIGHT/2).
     *  For instance:
     *
     * \code{.cpp}

       FillDir(pt, NORTH) // pt == (0, -2)
       FillDir(pt, SOUTHEAST) // pt == (2, 2)

     * \endcode
     *
     * @param pt The SPoint to fill with the offsets of a direction.
     *
     * @param dir The Dir specifying the units to fill \c pt with.
     *
     * @param isStaggered True when grid layout is staggered, false for checkerboard.
     */
    static void FillDir(SPoint& pt, Dir dir, bool isStaggered);

    /**
     * Given a Dir , will fill a SPoint with unit offsets representing
     * the direction of this Dir in Grid Coordinates. For instance:
     *
     * \code{.cpp}

       ToTileGridIndex(pt, NORTH) // pt == (0, -1)
       ToTileGridIndex(pt, SOUTHEAST) // pt == (1, 1)

     * \endcode
     *
     * @param pt The SPoint to fill with the offsets of a direction.
     *
     * @param dir The Dir specifying the units to fill \c pt with.
     *
     * @param isStaggered True when grid layout is staggered, false for checkerboard.
     *
     * @param fpt The from gridtilecoord as reference for staggered grids.
     */
    static void ToNeighborTileInGrid(SPoint & pt, u32 dir, bool isStaggered, const SPoint& fpt);

    /**
     * Translates the coordinates in a SPoint to a Dir . The
     * coordinates in SPoint must be a unit offset (like the ones
     * returned from \c FillDir ), otherwise this will FAIL with
     * ILLEGAL_ARGUMENT .
     *
     * @param pt The SPoint containing a unit offset to translate to a
     * Dir .
     *
     * @returns The Dir representing the offset held by \c pt .
     */
    static Dir FromOffset(SPoint& pt);

    /**
     * Flips the X coordinate of a given SPoint about the origin.
     *
     * @param pt The SPoint to flip the X coordinate of .
     *
     * @returns A reference to \c pt .
     */
    static inline SPoint & FlipXAxis(SPoint & pt) {
      pt.SetX(-pt.GetX());
      return pt;
    }


    /**
     * Flips the Y coordinate of a given SPoint about the origin.
     *
     * @param pt The SPoint to flip the Y coordinate of .
     *
     * @returns A reference to \c pt .
     */
    static inline SPoint & FlipYAxis(SPoint & pt) {
      pt.SetY(-pt.GetY());
      return pt;
    }

    /**
     * Flips the axes of a given SPoint about the origin. This behaves
     * as if the given SPoint is a SOUTHEAST SPoint, which will
     * represent the specified corner Dir once the flip has taken place. For instance,
     *
     * \code{.cpp}

       SPoint pt(1, 1); // Or, a SOUTHEAST SPoint
       FlipSEPointToCorner(pt, NORTHWEST); // pt is now a NORTHWEST SPoint, or (-1, -1)

     * \endcode This performs only axis flipping, keeping the distance
     * from the origin the same.
     *
     * @param pt The SPoint to flip the axes of.
     *
     * @param corner The corner representing the axes of \c pt to
     *               flip. This must be a corner (e.g. NORTHEAST,
     *               SOUTHEAST), else this method FAILs with
     *               ILLEGAL_ARGUMENT.
     *
     * @returns A new point containing the new coordinates of \c pt ,
     *          which is also changed.
     */
    static SPoint FlipSEPointToCorner(const SPoint& pt, const Dir corner);


  private:

    /**
     * private helpers for FillDir
     *
     */
    static void FillDirCheckerboard(SPoint& pt, u32 dir);
    static void FillDirStaggered(SPoint& pt, u32 dir);

    /**
     * private helpers for ToNeighborTileInGrid
     *
     */
    static void ToCheckerboardTileInGridNeighbor(SPoint& pt, u32 dir);
    static void ToStaggeredTileInGridNeighbor(SPoint& pt, u32 dir, const SPoint& fpt);

  };

  typedef RandomIterator<Dirs::DIR_COUNT,4> RandomDirIterator;

} /* namespace MFM */

#endif /*DIRS_H*/
