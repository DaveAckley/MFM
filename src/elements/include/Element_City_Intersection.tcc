/* -*- C++ -*- */

#include "Element_City_Street.h"

namespace MFM
{
  template <class CC>
  void Element_City_Intersection<CC>::InitializeIntersection(T& atom,
                                                             EventWindow<CC>& window) const
  {
    Random& rand = window.GetRandom();
    Dir d;
    SPoint offset;
    for(u32 i = 0; i < 8; i++)
    {
      d = (Dir)i;
      Dirs::FillDir(offset, d);

      if(rand.OneIn(10) &&
         (window.GetRelativeAtom(offset).GetType() ==
          Element_Empty<CC>::THE_INSTANCE.GetType()))
      {
        T newStreet = Element_City_Street<CC>::THE_INSTANCE.GetDefaultAtom();
        Element_City_Street<CC>::THE_INSTANCE.SetDirection(newStreet, d);
        window.SetRelativeAtom(offset, newStreet);
      }
    }
  }
}
