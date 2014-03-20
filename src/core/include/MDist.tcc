#include <stdio.h>         /* -*- C++ -*- */
#include "MDist.h"
#include "Fail.h"

namespace MFM {

  template<u32 R>
  MDist<R>::MDist()
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
  }

  template<u32 R>
  u32 MDist<R>::GetTableSize(u32 maxRadius)
  {
    return EVENT_WINDOW_SITES(maxRadius);
  }

  template<u32 R>
  MDist<R>& MDist<R>::get() {
    static MDist<R> instance;
    return instance;
  }


  template<u32 R>
  s32 MDist<R>::FromPoint(const Point<s32>& offset, u32 maxRadius)
  {
    u32 x = (u32) (offset.GetX()+R);
    u32 y = (u32) (offset.GetY()+R);
    if (x >= EVENT_WINDOW_DIAMETER || y >= EVENT_WINDOW_DIAMETER) 
      return -1;

    u32 idx = m_pointToIndex[x][y];

    // Ensure we're inside the allowed radius
    if (idx >= GetFirstIndex(maxRadius+1))
      return -1;

    return (s32) idx;
  }

  template<u32 R>
  void MDist<R>::FillFromBits(SPoint& pt, u8 bits, u32 maxRadius)
  {
    if (bits >= ARRAY_LENGTH)
      FAIL(ILLEGAL_ARGUMENT);

    Point<s32> & bp = m_indexToPoint[bits];

    pt.SetX(bp.GetX());
    pt.SetY(bp.GetY());
  }

    static SPoint VNNeighbors[4];
    

  template<u32 R>
  void MDist<R>::FillRandomSingleDir(SPoint& pt,Random & random)
  {
    switch(random.Create(4))
      {
      case 0: pt.Set(0, -1); break;
      case 1: pt.Set(0, 1);  break;
      case 2: pt.Set(-1, 0); break;
      case 3: pt.Set(1, 0); break;
      default: return;
      }
  }
} /* namespace MFM */

