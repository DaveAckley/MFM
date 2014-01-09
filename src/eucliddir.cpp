#include "eucliddir.h"

void EuDir::FillEuclidDir(Point<int>& pt, EuclidDir dir)
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
  }
}
