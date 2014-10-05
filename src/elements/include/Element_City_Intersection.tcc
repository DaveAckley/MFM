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
    for(u32 i = 0; i < Dirs::DIR_COUNT; i++)
    {
      d = (Dir)i;
      Dirs::FillDir(offset, d);

      if(window.GetRelativeAtom(offset).GetType() ==
          Element_Empty<CC>::THE_INSTANCE.GetType())
      {
        if(rand.OneIn(m_streetCreateOdds.GetValue()) && !(d & 1))
        {
          Element_City_Street<CC>& st = Element_City_Street<CC>::THE_INSTANCE;
          T newStreet = st.GetDefaultAtom();
          st.SetDirection(newStreet, d);
          window.SetRelativeAtom(offset, newStreet);
        }
        else
        {
          T newSidewalk = Element_City_Sidewalk<CC>::THE_INSTANCE.GetDefaultAtom();
          window.SetRelativeAtom(offset, newSidewalk);
        }
      }
    }
  }
}
