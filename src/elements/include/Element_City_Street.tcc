/* -*- C++ -*- */
#include "Element_City_Sidewalk.h"
#include "Element_City_Intersection.h"

namespace MFM
{
  template <class CC>
  void Element_City_Street<CC>::DoStreetAndSidewalk(EventWindow<CC>& window, Dir d) const
  {
    SPoint offset;
    Random& rand = window.GetRandom();

    Dirs::FillDir(offset, d);
    if((window.GetRelativeAtom(offset).GetType() ==
        Element_Empty<CC>::THE_INSTANCE.GetType()) &&
       rand.OneIn(m_intersectionOdds.GetValue()))
    {
      if(!CanSeeElementOfType(window, Element_City_Intersection<CC>::THE_INSTANCE.GetType()))
      {
        window.SetRelativeAtom(offset,
                               Element_City_Intersection<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }

    FillIfType(window, offset, Element_Empty<CC>::THE_INSTANCE.GetType(),
               window.GetCenterAtom());

    /* Don't do sidewalks if we can touch an intersection. */

    if(!MooreBorder(window, Element_City_Intersection<CC>::THE_INSTANCE.GetType()))
    {

      Dirs::FillDir(offset, Dirs::CCWDir(Dirs::CCWDir(d)));
      FillIfNotType(window, offset,
                    Element_City_Sidewalk<CC>::THE_INSTANCE.GetType(),
                    Element_City_Sidewalk<CC>::THE_INSTANCE.GetDefaultAtom());

      Dirs::FillDir(offset, Dirs::CWDir(Dirs::CWDir(d)));
      FillIfNotType(window, offset,
                    Element_City_Sidewalk<CC>::THE_INSTANCE.GetType(),
                    Element_City_Sidewalk<CC>::THE_INSTANCE.GetDefaultAtom());
    }

    /* Extra intersection creation */

    Dirs::FillDir(offset, d);
    if(window.GetRelativeAtom(offset).GetType() ==
       Element_City_Sidewalk<CC>::THE_INSTANCE.GetType())
    {
      window.SetRelativeAtom(offset * 2,
                             Element_City_Intersection<CC>::THE_INSTANCE.GetDefaultAtom());
      window.SetRelativeAtom(offset, window.GetCenterAtom());

    }
  }
}
