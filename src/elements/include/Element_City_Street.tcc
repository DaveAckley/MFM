/* -*- C++ -*- */
#include "Element_City_Sidewalk.h"
#include "Element_City_Intersection.h"

namespace MFM
{
  template <class CC>
  inline const typename CC::ATOM_TYPE& Element_City_Street<CC>::GetIntersection() const
  {
    return Element_City_Intersection<CC>::THE_INSTANCE.GetDefaultAtom();
  }

  template <class CC>
  inline u32 Element_City_Street<CC>::IntersectionType() const
  {
    return Element_City_Intersection<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  inline const typename CC::ATOM_TYPE& Element_City_Street<CC>::GetSidewalk() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetDefaultAtom();
  }

  template <class CC>
  inline u32 Element_City_Street<CC>::SidewalkType() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetType();
  }

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
      if(!CanSeeElementOfType(window, IntersectionType()))
      {
        window.SetRelativeAtom(offset, GetIntersection());
      }
    }

    FillIfType(window, offset, Element_Empty<CC>::THE_INSTANCE.GetType(),
               window.GetCenterAtom());

    /* Don't do sidewalks if we can touch an intersection. */

    if(!MooreBorder(window, IntersectionType()))
    {

      Dirs::FillDir(offset, Dirs::CCWDir(Dirs::CCWDir(d)));
      FillIfNotType(window, offset, SidewalkType(), GetSidewalk());

      Dirs::FillDir(offset, Dirs::CWDir(Dirs::CWDir(d)));
      FillIfNotType(window, offset, SidewalkType(), GetSidewalk());
    }

    /* Extra intersection creation */

    Dirs::FillDir(offset, d);
    if(window.GetRelativeAtom(offset).GetType() == SidewalkType())
    {
      if(window.GetRelativeAtom(offset * 2).GetType() == IntersectionType())
      {
        window.SetRelativeAtom(offset, window.GetCenterAtom());
      }
      else if(CanSeeElementOfType(window, IntersectionType()))
      {
        window.SetCenterAtom(GetSidewalk());
      }
      else
      {
        window.SetRelativeAtom(offset * 2, GetIntersection());
        window.SetRelativeAtom(offset, window.GetCenterAtom());
      }
    }
  }
}
