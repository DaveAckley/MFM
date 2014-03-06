#ifndef EUCLIDDIR_H      /* -*- C++ -*- */
#define EUCLIDDIR_H

#include "point.h"

namespace MFM {

typedef enum enum_eucliddir
{
  EUDIR_NORTH     = 0,
  EUDIR_NORTHEAST = 1,
  EUDIR_EAST      = 2,
  EUDIR_SOUTHEAST = 3,
  EUDIR_SOUTH     = 4,
  EUDIR_SOUTHWEST = 5,
  EUDIR_WEST      = 6,
  EUDIR_NORTHWEST = 7,
  EUDIR_COUNT
}EuclidDir;

class EuDir
{
public:
  /**
   * true iff dir is a corner direction.
   */
  static bool IsCorner(EuclidDir dir) { return dir&1; }

  /**
   * true iff dir is a face direction.
   */
  static bool IsFace(EuclidDir dir) { return !IsCorner(dir); }

  /**
   * The next dir clockwise from dir.  Note that for all dir,
   * IsCorner(dir)==IsFace(CWDir(dir)), and dir==CWDir(CCWDir(dir))
   */
  static EuclidDir CWDir(EuclidDir dir) { return (EuclidDir) ((dir+1)%EUDIR_COUNT); }

  /**
   * Gets the direction opposite the one specified.
   *
   * @returns The direction opposite the one specified.
   */
  static EuclidDir OppositeDir(EuclidDir dir)
  { return (EuclidDir)((dir + (EUDIR_COUNT / 2)) % EUDIR_COUNT); }

  /**
   * The next dir counter-clockwise from dir.  Note that for all dir,
   * IsCorner(dir)==IsFace(CCWDir(dir)), and dir==CCWDir(CWDir(dir))
   */
  static EuclidDir CCWDir(EuclidDir dir) { return (EuclidDir) ((dir+EUDIR_COUNT-1)%EUDIR_COUNT); }

  /**
   * Adds a EuclidDir to a EuclidDir mask.
   *
   * @param mask The EuclidDir mask to add a EuclidDir to.
   *
   * @param dir The EuclidDir to add to mask.
   *
   * @returns mask with dir added to it.
   */
  static inline u32 AddDirToMask(u32 mask, EuclidDir dir)
  { return mask | (1 << dir); }

  /**
   * Removes a EuclidDir to a EuclidDir mask.
   *
   * @param mask The EuclidDir mask to remove a EuclidDir from.
   *
   * @param dir The EuclidDir to remove from mask.
   *
   * @returns mask with dir removed from it.
   */
  static inline u32 RemoveDirInMask(u32 mask, EuclidDir dir)
  { return mask & (~(1 << dir)); }

  /**
   * Checks a EuclidDir mask to see if a EuclidDir is inside it.
   *
   * @param mask The mask to check EuclidDir membership of.
   *
   * @param dir The EuclidDir to check mask for membership of.
   *
   * @return true if dir is inside mask, else false.
   */
  static inline bool TestDirInMask(u32 mask, EuclidDir dir)
  { return mask & (1 << dir); }

  static void FillEuclidDir(SPoint& pt, EuclidDir dir);

  static EuclidDir FromOffset(SPoint& pt);
};

} /* namespace MFM */

#endif /*EUCLIDDIR_H*/
