#include <stdio.h>         /* -*- C++ -*- */
#include "MDist.h"
#include "Fail.h"
#include "Logger.h"

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
    {
      for (u32 y = 0; y<EVENT_WINDOW_DIAMETER; ++y)
      {
        m_pointToIndex[x][y] = -1;
      }
    }

    u32 next = 0;
    const SPoint center(R,R);

    // For every length from small to large
    for (u32 length = 0; length<=R; ++length)
    {
      m_firstIndex[length] = next;

      // Scan the whole event window
      for (u32 x = 0; x<EVENT_WINDOW_DIAMETER; ++x)
      {
        for (u32 y = 0; y<EVENT_WINDOW_DIAMETER; ++y)
	{
          Point<s32> p(x,y);
          p.Subtract(center);

          // And accumulate points of the given length
          if (p.GetManhattanLength()==length)
	  {
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

    InitEscapesByDirTable();
    InitHorizonsByDirTable();
  }

  template<u32 R>
  void MDist<R>::InitEscapesByDirTable()
  {
    /* Again, this is once-only and eventually to be pre-compiled into
       const, so for here and now let's see how unbelievably slow and
       obvious we can make this.
    */

    for (u32 d = Dirs::NORTH; d < Dirs::DIR_COUNT; ++d) // For each dir
    {
      // Get the delta to head in the given direction
      SPoint dirDelta;
      Dirs::FillDir(dirDelta, d); // Something in (-1 ,-1) to (1, 1)

      u32 countInDir = 0;       // Counts how many points we've stored for dir
      u32 deltasToEscape = 0;   // While gradually increasing this

      while (countInDir < ARRAY_LENGTH) // Until we're done for this dir
      {
        // On to next delta count
        ++deltasToEscape;

        // Rescan the entire event window, now looking for guys
        // exactly deltasToEscape away from escaping the event window,
        // if any, and add their indexes to the d direction of the
        // byDirection array.

        for (u32 idx = this->GetFirstIndex(0); idx <= this->GetLastIndex(R); ++idx)
        {
          SPoint rel = this->GetPoint(idx);

          // How many times do we have to add dirDelta to rel for it
          // to no longer be in the event window?

          u32 deltas;
          for (deltas = 1; deltas <= 2*R; ++deltas)
          {
            SPoint off = rel + dirDelta * deltas;
            if (off.GetManhattanLength() > R)
              break;
          }

          // If that number of deltas is the count we're looking for,
          // then rel is the next point('s idx) to capture.

          if (deltas == deltasToEscape)
          {
            m_escapesByDirection[d][countInDir++] = (u8) idx;
          }
        }
      }
    }
  }

  template<u32 R>
  void MDist<R>::InitHorizonsByDirTable()
  {
    /* Etc, ditto, unbelievably slow and obvious == good.
    */

    for (u32 d = Dirs::NORTH; d < Dirs::DIR_COUNT; ++d) // For each dir
    {
      // Get the delta to head in the given direction
      SPoint dirDelta;
      Dirs::FillDir(dirDelta, d); // Something in (-1 ,-1) to (1, 1)

      u32 countInDir = 0;       // Counts how many points we've stored for dir
      u32 deltasToHorizon = 0;  // While gradually increasing this

      while (countInDir < ARRAY_LENGTH) // Until we're done for this dir
      {
        // On to next delta count
        ++deltasToHorizon;

        // Rescan the entire event window, now looking for guys
        // exactly deltasToHorizon away from crossing the horizon line,
        // if any, and add their indexes to the d direction of the
        // byDirection array.

        // We want to alternate the horizon between even and odd so
        // the diagonals will iterate in alternating white and black
        // chess board strips, but we need to not do that for the
        // orthogonals, so we make this loop twice only for the
        // diagonals, which have a manhattan length of two.  (Well,
        // this surely ain't as obvious as hoped, but I'm not hanging
        // around to find the properly clever way.)

        for (u32 horizon = R + dirDelta.GetManhattanLength(); horizon >= R + 1; --horizon)
        {

          for (u32 idx = this->GetFirstIndex(0); idx <= this->GetLastIndex(R); ++idx)
          {
            SPoint rel = this->GetPoint(idx);

            // How many times do we have to add dirDelta to rel for it
            // reach the horizon?

            u32 deltas;
            for (deltas = 1; deltas <= 2*R; ++deltas)
            {
              // Multiply by abs(dirDelta) to knock out 0 components
              SPoint off(rel.GetX() * abs(dirDelta.GetX()), rel.GetY() * abs(dirDelta.GetY()));

              // Then scale whatever's left
              off += dirDelta * deltas;

              if (off.GetManhattanLength() == horizon)
                break;
            }

            // If that number of deltas is the count we're looking for,
            // then rel is the next point('s idx) to capture.

            if (deltas == deltasToHorizon)
            {
              // We found the next guy.
              m_horizonsByDirection[d][countInDir++] = (u8) idx;
              //LOG.Debug("%d %d (%d,%d)",d,countInDir,rel.GetX(), rel.GetY());
            }
          }
        }
      }
    }
  }

  template<u32 R>
  u32 MDist<R>::GetTableSize(u32 maxRadius) const
  {
    return EVENT_WINDOW_SITES(maxRadius);
  }

  template<u32 R>
  const MDist<R>& MDist<R>::get()
  {
    return THE_INSTANCE;
  }

  template<u32 R>
  s32 MDist<R>::FromPoint(const Point<s32>& offset, u32 maxRadius) const
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
  void MDist<R>::FillFromBits(SPoint& pt, u8 bits, u32 maxRadius) const
  {
    if (bits >= ARRAY_LENGTH)
      FAIL(ILLEGAL_ARGUMENT);

    const SPoint & bp = m_indexToPoint[bits];

    pt.SetX(bp.GetX());
    pt.SetY(bp.GetY());
  }

    static SPoint VNNeighbors[4];


  template<u32 R>
  void MDist<R>::FillRandomSingleDir(SPoint& pt,Random & random) const
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
