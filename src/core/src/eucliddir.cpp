#include "eucliddir.h"
#include "fail.h"


namespace MFM {
void EuDir::FillEuclidDir(SPoint& pt, EuclidDir dir)
{
  switch(dir)
  {
  case EUDIR_NORTH: pt.Set(0, -1); break;
  case EUDIR_EAST:  pt.Set(1,  0); break;
  case EUDIR_SOUTH: pt.Set(0,  1); break;
  case EUDIR_WEST:  pt.Set(-1, 0); break;
  case EUDIR_NORTHWEST:
    pt.Set(-1, -1); break;
  case EUDIR_NORTHEAST:
    pt.Set(1,  -1); break;
  case EUDIR_SOUTHEAST:
    pt.Set(1,   1); break;
  case EUDIR_SOUTHWEST:
    pt.Set(-1,  1); break;
  default:
    FAIL(ILLEGAL_ARGUMENT);
  }
}

EuclidDir EuDir::FromOffset(SPoint& pt)
{
  switch(pt.GetX())
  {
  case 1:
    switch(pt.GetY())
    {
    case 1:  return EUDIR_SOUTHEAST;
    case 0:  return EUDIR_EAST;
    case -1: return EUDIR_NORTHEAST;
    }
    break;
  case 0:
    switch(pt.GetY())
    {
    case 1:  return EUDIR_SOUTH;
    case -1: return EUDIR_NORTH;
    }
    break;
  case -1:
    switch(pt.GetY())
    {
    case 1:  return EUDIR_SOUTHWEST;
    case 0:  return EUDIR_WEST;
    case -1: return EUDIR_NORTHWEST;
    }
    break;
  }

  FAIL(ILLEGAL_ARGUMENT);
}
} /* namespace MFM */

