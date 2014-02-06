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
   * The next dir counter-clockwise from dir.  Note that for all dir,
   * IsCorner(dir)==IsFace(CCWDir(dir)), and dir==CCWDir(CWDir(dir))
   */
  static EuclidDir CCWDir(EuclidDir dir) { return (EuclidDir) ((dir+EUDIR_COUNT-1)%EUDIR_COUNT); }

  static void FillEuclidDir(SPoint& pt, EuclidDir dir);

  static EuclidDir FromOffset(SPoint& pt);
};

} /* namespace MFM */

#endif /*EUCLIDDIR_H*/
