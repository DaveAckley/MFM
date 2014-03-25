                               /* -*- C++ -*- */
#include "Fail.h" 
#include "Dirs.h"

namespace MFM
{
  template <class C>
  const SPoint Element<C>::VNNeighbors[4] = 
  {
      SPoint(-1, 0), SPoint(1, 0), SPoint(0, -1), SPoint(0, 1)
  };

  /* Fills 'pt' with the value of a randomly selected empty von neumann */
  /* neighbor.                                                          */
  /* Returns false if there is no valid neighbor to be used.            */
  template <class C>
  bool Element<C>::FillAvailableVNNeighbor(EventWindow<C>& window, SPoint& pt) const
  {
    return FillPointWithType(window, pt, VNNeighbors, 4, Dirs::SOUTHEAST, ELEMENT_EMPTY);
  }

  /* Master search method for finding atoms in regions in a window. If regions are
     symmetric about the origin, rotation does not make a difference. 
   */
  template <class C>
  bool Element<C>::FillPointWithType(EventWindow<C>& window, 
				       SPoint& pt, const SPoint* relevants, u32 relevantCount,
				       Dir rotation, ElementType type) const
  {
    Random & random = window.GetRandom();
    u32 possibles = 0;
    for(u32 i = 0; i < relevantCount; i++)
    {
      SPoint current = Dirs::FlipSEPointToCorner(relevants[i], rotation);
      
      /* Live site?  Right type?  Lucky? */
      if(window.IsLiveSite(current) &&
	 (window.GetRelativeAtom(current).GetType() == type) &&
         random.OneIn(++possibles))
        pt = current;
    }
    return possibles > 0;
  }

  template <class C>
  void Element<C>::Diffuse(EventWindow<C>& window) const
  {
    SPoint pt;
    if(FillAvailableVNNeighbor(window, pt))
    {
      window.SwapAtoms(pt, SPoint(0, 0));
    }
  }
}
