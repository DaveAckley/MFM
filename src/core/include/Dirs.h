/*                                              -*- mode:C++ -*-
  Dirs.h Euclidean direction system
  Copyright (C) 2014, 2016-2017 The Regents of the University of New Mexico.  All rights reserved.

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
  \date (C) 2014, 2016-2017 All rights reserved.
  \lgpl
 */
#ifndef DIRS_H
#define DIRS_H

#include "Point.h"
#include "Fail.h"

namespace MFM
{

  typedef u32 Dir;
  typedef u32 PrimaryDirIndex;

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

    /**
     * Given a Dir , will fill a SPoint with unit offsets representing
     * the direction of this Dir . For instance:
     *
     * \code{.cpp}

       FillDir(pt, NORTH) // pt == (0, -1)
       FillDir(pt, SOUTHEAST) // pt == (1, 1)

     * \endcode
     *
     * @param pt The SPoint to fill with the offsets of a direction.
     *
     * @param dir The Dir specifying the units to fill \c pt with.
     */
    static void FillDir(SPoint& pt, Dir dir);

    static SPoint GetOffset(Dir dir) {
      SPoint tmp;
      FillDir(tmp, dir);
      return tmp;
    }

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
  };

  typedef RandomIterator<Dirs::DIR_COUNT,4> RandomDirIterator;

  /**
     The relative positioning of adjacent tiles in a larger grid,
     determining which directions are legal at the intertile level,
     which and how many locks are needed for visible events, and
     remote tile origins relative to a given tile.
  */
  enum TileStagger {
    TILE_STAGGER_NONE,    //< Tile neighbors are N, NE, E, SE, S, SW, W, NW
    TILE_STAGGER_ROWS,    //< Tile neighbors are NW, NE, E, SE, SW, W
    TILE_STAGGER_COLUMNS, //< Tile neighbors are N, NE, SE, S, SW, NW
    MAX_TILE_STAGGERS
  };

  /**
     The number of direction pairs, worst-case.
   */
  static const u32 PRIMARY_DIR_INDEX_COUNT = 4;

  /**
     A minimal iterator over the 'directions' of a tile, visiting
     either all dirs or the 'primary' half of them.  Access via
     DirInTileIterator::BeginAll(TileStagger),
     DirInTileIterator::BeginPrimary(TileStagger),
     Tile::BeginAllDirs(), Tile::BeginPrimaryDirs()
  */
  class DirInTileIterator
  {
    const TileStagger ts;
    const bool all; // vs 'primary only'
    Dir dir;
    u32 count;
  public:
    static DirInTileIterator BeginAll(TileStagger ts) { return DirInTileIterator(ts, true, true); }
    static DirInTileIterator EndAll(TileStagger ts) { return DirInTileIterator(ts, true, false); }
    static DirInTileIterator BeginPrimary(TileStagger ts) { return DirInTileIterator(ts, false, true); }
    static DirInTileIterator EndPrimary(TileStagger ts) { return DirInTileIterator(ts, false, false); }

    DirInTileIterator(TileStagger stagger, bool allNotMaster, bool firstNotLast)
      : ts(stagger)
      , all(allNotMaster)
      , dir(firstNotLast ? GetFirstDir() : GetLastDir())
      , count(firstNotLast ? GetFirstCount() : GetLastCount())
    { }

    Dir GetFirstDir() const
    {
      switch (ts) {
      case TILE_STAGGER_NONE: return all ? Dirs::NORTH : Dirs::NORTHEAST;
      case TILE_STAGGER_ROWS: return Dirs::NORTHEAST;
      case TILE_STAGGER_COLUMNS:  return Dirs::NORTHEAST;
      default: FAIL(UNREACHABLE_CODE);
      }
    }

    Dir GetLastDir() const
    {
      switch (ts) {
      case TILE_STAGGER_NONE: return all ? Dirs::NORTHWEST : Dirs::SOUTH;
      case TILE_STAGGER_ROWS: return all ? Dirs::NORTHWEST : Dirs::SOUTHEAST;
      case TILE_STAGGER_COLUMNS:  return all ? Dirs::NORTH : Dirs::SOUTH;
      default: FAIL(UNREACHABLE_CODE);
      }
    }

    u32 GetFirstCount() const 
    {
      return 0;
    }

    u32 GetLastCount() const
    {
      switch (ts) {
      case TILE_STAGGER_NONE: return all ? Dirs::DIR_COUNT : Dirs::DIR_COUNT / 2;
      case TILE_STAGGER_ROWS:     // Fall through
      case TILE_STAGGER_COLUMNS:  return all ? 6 : 6 / 2;
      default: FAIL(UNREACHABLE_CODE);
      }
    }

    Dir GetNextDir(Dir cur) 
    {
      ++count;
      ++cur;
      switch (ts) {
      case TILE_STAGGER_NONE: break;
      case TILE_STAGGER_ROWS: if (cur == Dirs::NORTH || cur == Dirs::SOUTH) ++cur; break;
      case TILE_STAGGER_COLUMNS: if (cur == Dirs::EAST || cur == Dirs::WEST) ++cur; break;
      default: FAIL(UNREACHABLE_CODE);
      }
      return cur;
    }

    PrimaryDirIndex GetPrimaryDirIndex() const
    {
      MFM_API_ASSERT_STATE(!all);
      return count;
    }

    SPoint GetOffset(Dir cur) const
    {
      SPoint offset = Dirs::GetOffset(cur);
      SPoint ret = offset * 2; // switch to external doubled coords
      switch (ts) {
      case TILE_STAGGER_NONE: break;  // done
      case TILE_STAGGER_ROWS: if (cur&1) ret.SetX(offset.GetX()); break; // x not doubled on diagonals
      case TILE_STAGGER_COLUMNS: if (cur&1) ret.SetY(offset.GetY()); break; // y not doubled on diagonals
      default: FAIL(UNREACHABLE_CODE);
      }
      return ret;
    }

    bool operator!=(const DirInTileIterator &m) const
    {
      return dir != m.dir || ts != m.ts || all != m.all;
    }

    void operator++()
    {
      if (dir != GetLastDir()) 
        dir = GetNextDir(dir);
    }

    Dir operator*() const
    {
      return dir;
    }
    
    SPoint GetOffset() const
    {
      return GetOffset(dir);
    }

    u32 GetCount() const
    {
      return count;
    }
  };
} /* namespace MFM */

#endif /*DIRS_H*/
