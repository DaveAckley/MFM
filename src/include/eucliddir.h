#ifndef EUCLIDDIR_H      /* -*- C++ -*- */
#define EUCLIDDIR_H

#include "point.h"

typedef enum enum_eucliddir
{ 
  EUDIR_NORTH     = 0,
  EUDIR_NORTHEAST = 1,
  EUDIR_EAST      = 2,
  EUDIR_SOUTHEAST = 3,
  EUDIR_SOUTH     = 4,
  EUDIR_SOUTHWEST = 5,
  EUDIR_WEST      = 6,
  EUDIR_NORTHWEST = 7
}EuclidDir;

class EuDir
{
public:
  static void FillEuclidDir(Point<int>& pt, EuclidDir dir);
};

#endif /*EUCLIDDIR_H*/
