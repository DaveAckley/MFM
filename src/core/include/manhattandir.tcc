#include <stdio.h>         /* -*- C++ -*- */
#include "manhattandir.h"
#include "fail.h"

namespace MFM {

template<u32 R>
ManhattanDir<R>::ManhattanDir()
{
  /* This is once-only so let's be very stupid.  (We'll want to
     precompile all this out later anyway.)  We store the points in
     just one table, sorted by their lengths, and remember where the
     different lengths begin.  This lets us index and select offsets
     of any given length, or contiguous range of lengths, from zero up
     to the R. */

  /* Init the reverse lookup table to all 'illegal' */
  for (u32 x = 0; x<EVENT_WINDOW_DIAMETER; ++x) 
    for (u32 y = 0; y<EVENT_WINDOW_DIAMETER; ++y) 
      m_pointToIndex[x][y] = -1;

  u32 next = 0;
  const SPoint center(R,R);
  
  // For every length from small to large
  for (u32 length = 0; length<=R; ++length) {
    m_firstIndex[length] = next;

    // Scan the whole event window
    for (u32 x = 0; x<EVENT_WINDOW_DIAMETER; ++x) {
      for (u32 y = 0; y<EVENT_WINDOW_DIAMETER; ++y) {
        Point<s32> p(x,y);
        p.Subtract(center);

        // And accumulate points of the given length
        if (p.GetManhattanDistance()==length) {
          m_indexToPoint[next] = p;
          m_pointToIndex[x][y] = next;
          ++next;
        }
      }
    }
  }
  m_firstIndex[R+1] = next;
  if (next != EVENT_WINDOW_SITES(R))
    FAIL(ILLEGAL_STATE);

  /* Initialize the southeast table */
  u32 i = 0;
  for(s32 x = 1; (u32)x < R / 2 + 1; x++)
  {
    for(s32 y = 1; (u32)y < R / 2 + 1; y++)
    {
      m_southeastSubWindow[i++] = Point<s32>(x, y);
    }
  }
}

template<u32 R>
Point<s32>& ManhattanDir<R>::FlipAxis(Point<s32>& pt, bool xAxis)
{
  pt.Set(xAxis ? -pt.GetX() : pt.GetX(),
	 (!xAxis) ? -pt.GetY() : pt.GetY());

  return pt;
}

template<u32 R>
u32 ManhattanDir<R>::GetBondSize(TableType type)
{
  if (type < 0 || type > MANHATTAN_TABLE_EVENT)
    FAIL(ILLEGAL_ARGUMENT);
  return type;
}

template<u32 R>
u32 ManhattanDir<R>::GetTableSize(TableType type)
{

  u32 radius = GetBondSize(type);
  return EVENT_WINDOW_SITES(radius);
}

template<u32 R>
ManhattanDir<R>& ManhattanDir<R>::get() {
  static ManhattanDir<R> instance;
  return instance;
}


template<u32 R>
s32 ManhattanDir<R>::FromPoint(const Point<s32>& offset, TableType type)
{
  u32 x = (u32) (offset.GetX()+R);
  u32 y = (u32) (offset.GetY()+R);
  if (x >= EVENT_WINDOW_DIAMETER || y >= EVENT_WINDOW_DIAMETER) 
    return -1;

  u32 idx = m_pointToIndex[x][y];

  // Ensure we're inside the requested table size
  if (idx >= m_firstIndex[type+1])
    return -1;

  return (s32) idx;
}

template<u32 R>
void ManhattanDir<R>::FillFromBits(SPoint& pt,
				u8 bits, TableType type)
{
  if (bits >= ARRAY_LENGTH)
    FAIL(ILLEGAL_ARGUMENT);

  Point<s32> & bp = m_indexToPoint[bits];

  pt.SetX(bp.GetX());
  pt.SetY(bp.GetY());
}

template<u32 R>
void ManhattanDir<R>::FillVNNeighbors(SPoint* pts)
{
  pts[0].Set(-1, 0);
  pts[1].Set(0, -1);
  pts[2].Set(1,  0);
  pts[3].Set(0,  1);
}

template<u32 R>
void ManhattanDir<R>::FillRandomSingleDir(SPoint& pt)
{
  switch(rand() & 3)
  {
  case 0: pt.Set(0, -1); break;
  case 1: pt.Set(0, 1);  break;
  case 2: pt.Set(-1, 0); break;
  case 3: pt.Set(1, 0); break;
  default: return;
  }
}
} /* namespace MFM */

