/* -*- C++ -*- */

#include "Element_City_Building.h"

namespace MFM
{
  template <class CC>
  void Element_City_Sidewalk<CC>::DoBuildingBehavior(EventWindow<CC>& window) const
  {
    MDist<R>& md = MDist<R>::get();
    /* Find an empty spot next to me and make a building there. */
    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(1); i++)
    {
      SPoint pt = md.GetPoint(i);

      if(window.GetRelativeAtom(pt).GetType() == Element_Empty<CC>::THE_INSTANCE.GetType())
      {
        if(!CanSeeElementOfType(window, Element_City_Building<CC>::THE_INSTANCE.GetType(), 1))
        {
          /* Found it! */
          window.SetRelativeAtom(pt, Element_City_Building<CC>::THE_INSTANCE.GetDefaultAtom());
          break;
        }
      }
    }
  }
}
