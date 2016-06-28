/* -*- C++ -*- */
#include "Element_Empty.h"

namespace MFM
{
  namespace ElementUtils
  {

    /* Fills 'pt' with the value of a randomly selected empty von neumann */
    /* neighbor.                                                          */
    /* Returns false if there is no valid neighbor to be used.            */
    template <class EC>
    bool FillAvailableVNNeighbor(EventWindow<EC>& window, SPoint& pt)
    {
      return FillPointWithType(window, pt, VNNeighbors, 4, Dirs::SOUTHEAST, EC::ATOM_CONFIG::ATOM_EMPTY_TYPE);
    }

    /* Master search method for finding atoms in regions in a window. If regions are
       symmetric about the origin, rotation does not make a difference.
    */
    template <class EC>
    bool FillPointWithType(EventWindow<EC>& window,
                           SPoint& pt, const SPoint* relevants, u32 relevantCount,
                           Dir rotation, ElementType type)
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
            {
              pt = current;
            }
        }
      return possibles > 0;
    }
  }
}
