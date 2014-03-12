#include "fail.h" /* -*- C++ -*- */

namespace MFM
{
  template <class T, u32 R>
  SPoint Element<T,R>::VNNeighbors[4] = 
  {
      SPoint(-1, 0), SPoint(1, 0), SPoint(0, -1), SPoint(0, 1)
  };

  /* Fills 'pt' with the value of a randomly selected empty von neumann */
  /* neighbor.                                                          */
  /* Returns false if there is no valid neighbor to be used.            */
  template <class T, u32 R>
  bool Element<T,R>::FillAvailableVNNeighbor(EventWindow<T,R>& window, SPoint& pt) const
  {
    return FillPointWithType(window, pt, VNNeighbors, 4, EUDIR_SOUTHEAST, ELEMENT_NOTHING);
  }

  template <class T, u32 R>
  void Element<T,R>::FlipSEPointToCorner(SPoint& readPt, SPoint& outPt, EuclidDir corner) const
  {
    outPt = readPt;
    switch(corner)
    {
    case EUDIR_SOUTHEAST: break;
    case EUDIR_NORTHEAST: ManhattanDir<R>::get().FlipAxis(outPt, false); break;
    case EUDIR_SOUTHWEST: ManhattanDir<R>::get().FlipAxis(outPt, true); break;
    case EUDIR_NORTHWEST:
      ManhattanDir<R>::get().FlipAxis(outPt, true);
      ManhattanDir<R>::get().FlipAxis(outPt, false);
      break;
    default: FAIL(ILLEGAL_ARGUMENT); break;
    }
  }

  /* Master search method for finding atoms in regions in a window. If regions are
     symmetric about the origin, rotation does not make a difference. 
   */
  template <class T, u32 R>
  bool Element<T,R>::FillPointWithType(EventWindow<T,R>& window, 
				       SPoint& pt, SPoint* relevants, u32 relevantCount,
				       EuclidDir rotation, ElementType type) const
  {
    Tile<T,R>& tile = window.GetTile();
    const SPoint& center = window.GetCenter();
    SPoint current(0, 0);
    SPoint relevantFlip;
    u32 possibles = 0;
    for(u32 i = 0; i < relevantCount; i++)
    {
      FlipSEPointToCorner(relevants[i], relevantFlip, rotation);
      current = center + relevantFlip;
      
      /* Dead cache? Not of right type? */
      if((tile.IsInCache(current) && !tile.IsConnected(tile.CacheAt(current))) ||
	 (window.GetRelativeAtom(relevantFlip).GetType() != type))
      {
	continue;
      }
      else
      {
	possibles++;
	if(tile.GetRandom().OneIn(possibles))
        {
	  pt = relevantFlip;
	}
      }
    }
    return possibles > 0;
  }

  template <class T, u32 R>
  void Element<T,R>::Diffuse(EventWindow<T,R>& window) const
  {
    SPoint pt;
    if(FillAvailableVNNeighbor(window, pt))
    {
      window.SwapAtoms(pt, SPoint(0, 0));
    }
  }
}
