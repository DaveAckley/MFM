/* -*- C++ -*- */

#include "Element_City_Building.h"

namespace MFM
{
  template <class EC>
  void Element_City_Sidewalk<EC>::DoBuildingBehavior(EventWindow<EC>& window) const
  {
    WindowScanner<EC> scanner(window);
    SPoint emptyPt;
    if(scanner.FindEmptyInVonNeumann(emptyPt) > 0)
    {
      window.SetRelativeAtomDirect(emptyPt,
                             Element_City_Building<EC>::THE_INSTANCE.GetDefaultAtom());
    }
  }

  template <class EC>
  u32 Element_City_Sidewalk<EC>::GetBuildingType() const
  {
    return Element_City_Building<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Sidewalk<EC>::GetStreetType() const
  {
    return Element_City_Street<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  void Element_City_Sidewalk<EC>::UpdateBuildingCounts(EventWindow<EC>& window) const
  {
    /* Find a random sidewalk I'm directly next to */
    WindowScanner<EC> scanner(window);
    SPoint borderSidewalk;
    u32 sidewalkCount;
    SPoint borderBuilding;
    u32 buildingCount;
    SPoint borderStreet;
    u32 streetCount;

    scanner.FindRandomAtoms(1, 3,
                            &borderSidewalk, TYPE(), &sidewalkCount,
                            &borderBuilding, GetBuildingType(), &buildingCount,
                            &borderStreet, GetStreetType(), &streetCount);

    if(buildingCount > 0)
    {
      /* Set my distance from this building to 0 */
      T newMe = window.GetCenterAtomDirect();
      SetBuildingCount(newMe,
                       Element_City_Building<EC>::THE_INSTANCE.
                           GetSubType(window.GetRelativeAtomDirect(borderBuilding)),
                       0);

      window.SetCenterAtomDirect(newMe);
    }

    if(sidewalkCount > 0)
    {
      if(IsReadyToBuild(window.GetRelativeAtomDirect(borderSidewalk)))
      {
        u32 count = 0;
        for(u32 i = 0; i < CityConstants::CITY_BUILDING_COUNT; i++)
        {
          count = MIN(GetBuildingCount(window.GetCenterAtomDirect(), i),
                      GetBuildingCount(window.GetRelativeAtomDirect(borderSidewalk), i));

          T newMe = window.GetCenterAtomDirect();
          SetBuildingCount(newMe, i, count);
          window.SetCenterAtomDirect(newMe);
        }
      }
    }

    if(streetCount > 0)
    {
      /* See if there's a sidewalk on the other side of the street */
      borderStreet *= 2;

      if(window.IsLiveSiteDirect(borderStreet))
      {
        if(window.GetRelativeAtomDirect(borderStreet).GetType() == TYPE())
        {
          if(IsReadyToBuild(window.GetRelativeAtomDirect(borderStreet)))
          {
            /* Sure is! Do the min trick, but add 1 to its counts. */
            u32 count;
            for(u32 i = 0; i < CityConstants::CITY_BUILDING_COUNT; i++)
            {
              count = MIN(GetBuildingCount(window.GetCenterAtomDirect(), i),
                          GetBuildingCount(window.GetRelativeAtomDirect(borderStreet), i) + 1);

              if(count < 4) /* Can't store higher res than 2 bits */
              {
                T newMe = window.GetCenterAtomDirect();
                SetBuildingCount(newMe, i, count);
                window.SetCenterAtomDirect(newMe);
              }
            }
          }
	  if (IsRebuilding(window.GetRelativeAtomDirect(borderStreet)) &&
	      (GetBuildingTimer(window.GetRelativeAtomDirect(borderStreet)) <
	       MAX_TIMER_VALUE * 0.1)) {
	    T newCen = window.GetCenterAtomDirect();
	    SetRebuildFlag(newCen);
	    window.SetCenterAtomDirect(newCen);
	  }
        }
      }
    }
  }
}
