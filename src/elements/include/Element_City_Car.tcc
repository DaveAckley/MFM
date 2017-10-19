/* -*- C++ -*- */

#include "Element_City_Building.h"
#include "Element_City_Street.h"
#include "Element_City_Intersection.h"

namespace MFM
{

  template <class EC>
  u32 Element_City_Car<EC>::GetBuildingType() const
  {
    return Element_City_Building<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Car<EC>::GetStreetType() const
  {
    return Element_City_Street<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Car<EC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Car<EC>::GetIntersectionType() const
  {
    return Element_City_Intersection<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  void Element_City_Car<EC>::ReplaceCenterWithStreet(EventWindow<EC>& window) const
  {
    T newStreet = Element_City_Street<EC>::THE_INSTANCE.GetDefaultAtom();
    Element_City_Street<EC>::THE_INSTANCE.
    SetDirection(newStreet,
                 GetDirection(window.GetCenterAtomDirect()));
    window.SetCenterAtomDirect(newStreet);
  }

  template <class EC>
  void Element_City_Car<EC>::Behavior(EventWindow<EC>& window) const
  {
    SPoint heading;
    T newMe;
    WindowScanner<EC> scanner(window);
    bool isStaggered = scanner.IsScannerGridLayoutStaggered();

    Dirs::FillDir(heading, GetDirection(window.GetCenterAtomDirect()), isStaggered);

    newMe = window.GetCenterAtomDirect();

    if(UseGas(newMe))
    {
      ReplaceCenterWithStreet(window);
      LOG.Debug("Out of gas!");

      return;
    }
    else
    {
      window.SetCenterAtomDirect(newMe);
    }

    if(!window.IsLiveSiteDirect(heading) ||
       window.GetRelativeAtomDirect(heading).GetType() == GetSidewalkType())
    {
      /* Can't move there? Turn around and wait for another
       * event.*/
      SetDirection(newMe, Dirs::OppositeDir(GetDirection(newMe)));
      window.SetCenterAtomDirect(newMe);
    }
    else
    {
      SPoint buildings[2];
      Dirs::FillDir(buildings[0],
                    Dirs::CWDir(Dirs::CWDir(GetDirection(window.GetCenterAtomDirect()))), isStaggered);
      Dirs::FillDir(buildings[1],
                    Dirs::CCWDir(Dirs::CCWDir(GetDirection(window.GetCenterAtomDirect()))), isStaggered);

      for(u32 i = 0; i < 2; i++)
      {
        buildings[i] *= 2; /* Was pointing at sidewalk */
        if(window.GetRelativeAtomDirect(buildings[i]).GetType() == GetBuildingType())
        {
          if(Element_City_Building<EC>::THE_INSTANCE.GetSubType(
                 window.GetRelativeAtomDirect(buildings[i])) ==
             GetDestType(window.GetCenterAtomDirect()))
          {
            /* Found a building! Finally. Report our gas usage. */
            LOG.Debug("Gas Usage: %d", MAX_GAS - GetGas(window.GetCenterAtomDirect()));
            ReplaceCenterWithStreet(window);
            return;
          }
        }
      }

      /* What if I'm at an intersection and the intersection borders
         the  building I want to be at? Let's get consumed there too. */

      SPoint intersectionPt;
      if(scanner.FindRandomInVonNeumann(GetIntersectionType(),
                                        intersectionPt) > 0)
      {
        const MDist<R>& md = MDist<R>::get();
        for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(1); i++)
        {
          SPoint mdp = (md.GetPoint(i) * 2) + intersectionPt;
          if(window.GetRelativeAtomDirect(mdp).GetType() == GetBuildingType())
          {
            if(Element_City_Building<EC>::THE_INSTANCE.GetSubType(
                                          window.GetRelativeAtomDirect(mdp)) ==
               GetDestType(window.GetCenterAtomDirect()))
            {
              /* Found a building! Finally. Report our gas usage. */
              LOG.Debug("Gas Usage: %d", GetGas(window.GetCenterAtomDirect()));
              ReplaceCenterWithStreet(window);
              return;
            }
          }
        }
      }

      if(window.GetRelativeAtomDirect(heading).GetType() == TYPE() ||
         window.GetRelativeAtomDirect(heading).GetType() == GetStreetType())
      {
        /* We can move! swap. */
        window.SwapAtomsDirect(SPoint(0, 0), heading);
      }
    }
  }
}
