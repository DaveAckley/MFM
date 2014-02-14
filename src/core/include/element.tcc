#include "fail.h" /* -*- C++ -*- */
#include "eucliddir.h"

namespace MFM
{
  template <class T, u32 R>
  void Element<T,R>::ReproduceVertically(EventWindow<T,R>& window, StateFunction f,
			   ElementType type)
  {
    Random & random = window.GetRandom();
    
    u32 cval = window.GetCenterAtom().ReadLowerBits();
    bool down = random.CreateBool();
    SPoint repPt(0, down ? R/2 : -(R/2));
    if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
    {
      window.SetRelativeAtom(repPt, T(type));
      window.GetRelativeAtom(repPt).WriteLowerBits(cval + (down ? 1 : -1));
    }
  }

  static SPoint VNNeighbors[4] =
  {
    SPoint(-1, 0), SPoint(1, 0), SPoint(0, -1), SPoint(0, 1)
  };
  
  /* Fills 'points' with all points relative to 'window's center atom which */
  /* are empty.                                                             */

  /* Returns the number of available VNNeighbors found.                     */
  template <class T, u32 R>
  u32 Element<T,R>::FillAvailableVNNeighbors(EventWindow<T,R>& window, StateFunction f,
					      SPoint* points)
  {
    Tile<T,R> tile = window.GetTile();
    SPoint& center = window.GetCenter();
    SPoint current(0, 0);
    u32 ptIdx = 0;
    for(u32 i = 0; i < 4; i++)
    {
      current = center;
      current.Add(VNNeighbors[i]);

      /* Is it in a dead cache or not an ELEMENT_NOTHING? */
      if((tile.IsInCache(current) && !tile.IsConnected(EuDir::FromOffset(VNNeighbors[i]))) ||
	 (f(&window.GetRelativeAtom(VNNeighbors[i])) != ELEMENT_NOTHING))
      {
	continue;
      }
      else
      {
	points[ptIdx++] = VNNeighbors[i];
      }
    }
    points[ptIdx].Set(0, 0);
    return ptIdx;
  }

  template <class T, u32 R>
  void Element<T,R>::Diffuse(EventWindow<T,R>& window, StateFunction f)
  {
    SPoint avails[4];
    u32 openSpots = FillAvailableVNNeighbors(window, f, avails);
    if(openSpots)
    {
      u32 spotToPick =  window.GetTile().GetRandom().Between(0, openSpots);
      window.SwapAtoms(avails[spotToPick], SPoint(0, 0));
    }
  }
}
