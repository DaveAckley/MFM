/* -*- C++ -*- */
#include "Element_City_Sidewalk.h"
#include "Element_City_Intersection.h"

namespace MFM
{
  template <class EC>
  inline const typename EC::ATOM_CONFIG::ATOM_TYPE& Element_City_Street<EC>::GetIntersection() const
  {
    return Element_City_Intersection<EC>::THE_INSTANCE.GetDefaultAtom();
  }

  template <class EC>
  inline u32 Element_City_Street<EC>::IntersectionType() const
  {
    return Element_City_Intersection<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  inline const typename EC::ATOM_CONFIG::ATOM_TYPE& Element_City_Street<EC>::GetSidewalk() const
  {
    return Element_City_Sidewalk<EC>::THE_INSTANCE.GetDefaultAtom();
  }

  template <class EC>
  inline u32 Element_City_Street<EC>::SidewalkType() const
  {
    return Element_City_Sidewalk<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  inline u32 Element_City_Street<EC>::BuildingType() const
  {
    return Element_City_Building<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  void Element_City_Street<EC>::DoStreetAndSidewalk(EventWindow<EC>& window, Dir d) const
  {
    SPoint offset;
    Random& rand = window.GetRandom();

    Dirs::FillDir(offset, d, false);
    offset /= 2;
    if((window.GetRelativeAtomDirect(offset).GetType() ==
        Element_Empty<EC>::THE_INSTANCE.GetType()) &&
       rand.OneIn(m_intersectionOdds.GetValue()))
    {
      if(!CanSeeElementOfType(window, IntersectionType()))
      {
        window.SetRelativeAtomDirect(offset, GetIntersection());
      }
    }

    FillIfType(window, offset, Element_Empty<EC>::THE_INSTANCE.GetType(),
               window.GetCenterAtomDirect());

    /* Don't do sidewalks if we can touch an intersection. */

    if(!MooreBorder(window, IntersectionType()))
    {
      Dirs::FillDir(offset, Dirs::CCWDir(Dirs::CCWDir(d)), false);
      offset /= 2;
      FillIfNotType(window, offset, SidewalkType(), GetSidewalk());

      Dirs::FillDir(offset, Dirs::CWDir(Dirs::CWDir(d)), false);
      offset /= 2;
      FillIfNotType(window, offset, SidewalkType(), GetSidewalk());
    }

    /* Extra intersection creation */

    Dirs::FillDir(offset, d, false);
    offset /= 2;
    if(window.GetRelativeAtomDirect(offset).GetType() == SidewalkType())
    {
      if(window.GetRelativeAtomDirect(offset * 2).GetType() == IntersectionType())
      {
        window.SetRelativeAtomDirect(offset, window.GetCenterAtomDirect());
      }
      else if(CanSeeElementOfType(window, IntersectionType()))
      {
        window.SetCenterAtomDirect(GetSidewalk());
      }
      else
      {
        window.SetRelativeAtomDirect(offset * 2, GetIntersection());
        window.SetRelativeAtomDirect(offset, window.GetCenterAtomDirect());
      }
    }

    /* If a building is in front of us, become a sidewalk. */
    if(window.GetRelativeAtomDirect(offset).GetType() == BuildingType())
    {
      window.SetCenterAtomDirect(GetSidewalk());
    }
  }
}
