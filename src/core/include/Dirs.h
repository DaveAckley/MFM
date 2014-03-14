#ifndef DIRS_H      /* -*- C++ -*- */
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
  };

} /* namespace MFM */

#endif /*DIRS_H*/
