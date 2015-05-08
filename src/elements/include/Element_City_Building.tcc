/* -*- C++ -*- */

#include "Element_City_Building.h"
#include "Element_City_Car.h"
#include "Element_City_Intersection.h"
#include "Element_City_Street.h"
#include "Element_City_Sidewalk.h"

namespace MFM
{
  template <class EC>
  u32 Element_City_Building<EC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Building<EC>::GetStreetType() const
  {
    return Element_City_Street<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Building<EC>::GetIntersectionType() const
  {
    return Element_City_Intersection<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Building<EC>::GetCarType() const
  {
    return Element_City_Car<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  bool Element_City_Building<EC>::DoNBSidewalkCase(EventWindow<EC>& window) const
  {
    SPoint sidewalk;
    WindowScanner<EC> scanner(window);
    u32 sidewalkCount = scanner.FindRandomInVonNeumann(GetSidewalkType(), sidewalk);

    if(sidewalkCount == 1)
    {
      Dir swDir = Dirs::FromOffset(sidewalk);

      SPoint adjs[2];
      Dirs::FillDir(adjs[0], Dirs::CWDir(Dirs::CWDir(swDir)));
      Dirs::FillDir(adjs[1], Dirs::CCWDir(Dirs::CCWDir(swDir)));

      for(u32 i = 0; i < 2; i++)
      {
        const T& at = window.GetRelativeAtomDirect(adjs[i]);
        if(at.GetType() == Element_Empty<EC>::THE_INSTANCE.GetType())
        {
          T copyMe = window.GetCenterAtomDirect();
          u32 idx = LargestVisibleIndex(window) + 1;

          const MDist<R>& md = MDist<R>::get();
          bool foundSidewalk = false;
          for(u32 j = md.GetFirstIndex(1); j <= md.GetLastIndex(1); j++)
          {
            if(window.GetRelativeAtomDirect(md.GetPoint(j) + adjs[i]).GetType() ==
               GetSidewalkType())
            {
              if(Element_City_Sidewalk<EC>::THE_INSTANCE.
                 IsReadyToBuild(window.GetRelativeAtomDirect(md.GetPoint(j) + adjs[i])))
              {
                foundSidewalk = true;
                break;
              }
            }
          }

          if(idx <= GetMaxArea(copyMe) && foundSidewalk)
          {
            SetAreaIndex(copyMe, idx);
            window.SetRelativeAtomDirect(adjs[i], copyMe);
            return true;
          }
        }
      }
    }
    return false;
  }

  template <class EC>
  bool Element_City_Building<EC>::DoNBPerpGrowthCase(EventWindow<EC>& window) const
  {
    SPoint swPoint;
    WindowScanner<EC> scanner(window);
    u32 sidewalkCount = scanner.FindRandomInVonNeumann(GetSidewalkType(), swPoint);

    if(window.GetRandom().OneIn(10))
    {
      if(sidewalkCount == 1)
      {
        SPoint oppositePt;
        Dir swDir = Dirs::FromOffset(swPoint);
        Dirs::FillDir(oppositePt, Dirs::OppositeDir(swDir));

        if(window.GetRelativeAtomDirect(oppositePt).GetType() ==
           Element_Empty<EC>::THE_INSTANCE.GetType())
        {
          T copyMe = window.GetCenterAtomDirect();
          u32 idx = LargestVisibleIndex(window) + 1;

          if(idx <= GetMaxArea(copyMe))
          {
            SetAreaIndex(copyMe, idx);
            window.SetRelativeAtomDirect(oppositePt, copyMe);
            return true;
          }
        }
      }
    }
    return false;
  }

  template <class EC>
  bool Element_City_Building<EC>::DoNoSidewalkConsume(EventWindow<EC>& window) const
  {
    WindowScanner<EC> scanner(window);

    if (scanner.IsBorderingVonNeumann(GetSidewalkType()) &&
       (!scanner.CanSeeAtomOfType(GetStreetType(), 2)) &&
       (!scanner.CanSeeAtomOfType(GetIntersectionType(), 2)) &&
       (!scanner.CanSeeAtomOfType(GetCarType(), 2)))
    {
      /* We're technically not on the street at this point. Let's get
       * changed to a rebuilding sidewalk. */

      T newCen = Element_City_Sidewalk<EC>::THE_INSTANCE.GetDefaultAtom();
      Element_City_Sidewalk<EC>::THE_INSTANCE.SetRebuildFlag(newCen);
      window.SetCenterAtomDirect(newCen);
    }
    return true;
  }

  template <class EC>
  void Element_City_Building<EC>::SpawnNextBuilding(EventWindow<EC>& window) const
  {
    if(!DoNBSidewalkCase(window))
    {
      if(!DoNBPerpGrowthCase(window))
      {
        DoNoSidewalkConsume(window);
      }
    }
  }

  template <class EC>
  typename EC::ATOM_CONFIG::ATOM_TYPE Element_City_Building<EC>::MakeRandomCar(u32 myBuildingType,
                                                                  Random& rand) const
  {
    u32 type;
    T car = Element_City_Car<EC>::THE_INSTANCE.GetDefaultAtom();
    do
    {
      type = rand.Create(CityConstants::CITY_BUILDING_COUNT);
    }while(type == myBuildingType);

    Element_City_Car<EC>::THE_INSTANCE.SetDestType(car, type);
    return car;
  }

  template <class EC>
  void Element_City_Building<EC>::SpawnCar(EventWindow<EC>& window) const
  {
    WindowScanner<EC> scanner(window);
    SPoint streetPt;

    if(scanner.FindRandomLocationOfType(GetStreetType(), streetPt))
    {
      /* Make a car with a random destination. Because we want
       * routing to be interesting, let's avoid spawning it with our
       * building type.*/
      T car = MakeRandomCar(GetSubType(window.GetCenterAtomDirect()), window.GetRandom());

      Dir carDir = Element_City_Street<EC>::THE_INSTANCE.GetDirection(
                       window.GetRelativeAtomDirect(streetPt));

      if(window.GetRandom().CreateBool())
      {
        carDir = Dirs::OppositeDir(carDir);
      }

      Element_City_Car<EC>::THE_INSTANCE.SetDirection(car, carDir);

      window.SetRelativeAtomDirect(streetPt, car);
    }
  }
}
