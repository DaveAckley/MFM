/* -*- C++ -*- */

#include "Element_City_Street.h"
#include "Element_City_Sidewalk.h"
#include "Element_City_Car.h"

namespace MFM
{
  template <class EC>
  u32 Element_City_Intersection<EC>::GetStreetType() const
  {
    return Element_City_Street<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Intersection<EC>::GetCarType() const
  {
    return Element_City_Car<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Intersection<EC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<EC>::THE_INSTANCE.GetType();
  }

  template <class EC>
  u32 Element_City_Intersection<EC>::IsRelDirCarOrStreet(EventWindow<EC> window,
                                                         Dir d) const
  {
    SPoint dp;
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();
    Dirs::FillDir(dp, d, isStaggered);
    const T& atom = window.GetRelativeAtom(dp);

    return atom.GetType() == GetCarType() || atom.GetType() == GetStreetType();
  }

  template <class EC>
  bool Element_City_Intersection<EC>::CanalIsValid(EventWindow<EC>& window,
                                                   u32 destType) const
  {
    Dir canal = GetCanalDir(window.GetCenterAtom(), destType);
    SPoint canalPt;
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();
    Dirs::FillDir(canalPt, canal, isStaggered);
    return window.GetRelativeAtom(canalPt).GetType() == GetStreetType();
  }

  template <class EC>
  Dir Element_City_Intersection<EC>::FindRandomRoute(EventWindow<EC>& window) const
  {
    return (Dir)((window.GetRandom().Create(4)) * 2);
  }

  template <class EC>
  Dir Element_City_Intersection<EC>::FindBestRouteCanal(EventWindow<EC>& window,
                                                        u32 destinationType,
                                                        Dir comingFrom) const
  {
    Dir canal = GetCanalDir(window.GetCenterAtom(), destinationType);
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();

    /* First, see if the canal will help get him in the right
     * direction.  If the canal points in a direction that he isn't
     * coming from, route him towards the canal.
     */

    if (CanalIsValid(window, destinationType) &&
        canal != Dirs::OppositeDir(comingFrom))
    {
      return canal;
    }


    /*
     * The canal seems to have been wrong... Let's go ahead and find
     * another direction and set the canal accordingly.
     */

    u32 roadFitness[] = {100, 100, 100};
    SPoint roads[3];
    Dir dirs[3]; /* The non-canal directions */
    u32 i = 0;
    for (Dir d = Dirs::NORTH; d < Dirs::DIR_COUNT; d += 2)
    {
      if(d != Dirs::OppositeDir(comingFrom))
      {
        dirs[i] = d;
        Dirs::FillDir(roads[i], dirs[i], isStaggered);
        i++;
      }
    }

    for(u32 i = 0; i < 3; i++)
    {
      if(window.IsLiveSite(roads[i]))
      {
        if(window.GetRelativeAtom(roads[i]).GetType() != GetStreetType() &&
           window.GetRelativeAtom(roads[i]).GetType() != GetCarType())
        {
          roadFitness[i] += 100; /* That isn't a road! */
        }
        else
        {
          for(u32 j = 0; j < 2; j++)
          {
            SPoint edgeSidewalk;
            if(j == 0)
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CWDir(Dirs::CWDir(dirs[i])), isStaggered);
            }
            else
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CCWDir(Dirs::CCWDir(dirs[i])), isStaggered);
            }

            edgeSidewalk = edgeSidewalk + roads[i];
            if(window.GetRelativeAtom(edgeSidewalk).GetType() !=
               Element_City_Sidewalk<EC>::THE_INSTANCE.GetType())
            {
              roadFitness[i] += 100; /* Don't want to go that way! */
            }
            else
            {
              if(Element_City_Sidewalk<EC>::THE_INSTANCE.
                 IsReadyToBuild(window.GetRelativeAtom(edgeSidewalk)))
              {
                roadFitness[i] -=
                3 - (
                Element_City_Sidewalk<EC>::THE_INSTANCE.
                GetBuildingCount(window.
                                 GetRelativeAtom(edgeSidewalk), destinationType));
              }
              else
              {
                roadFitness[i]++;
              }
            }
          }
        }
      }
    }

    u32 bestDirValue = MIN(roadFitness[0],
                           MIN(roadFitness[1], roadFitness[2]));
    u32 bestCount = 0;
    Dir dirsOfMin[3];

    for(u32 i = 0; i < 3; i++)
    {
      if(roadFitness[i] == bestDirValue)
      {
        dirsOfMin[bestCount++] = dirs[i];
      }
    }

    Dir bestDir = dirsOfMin[window.GetRandom().Create(bestCount)];

    /* Now that we have this, reassign the canal. */
    T newAtom = window.GetCenterAtom();
    SetCanalDir(newAtom, destinationType, bestDir);
    window.SetCenterAtom(newAtom);

    return bestDir;
  }


  template <class EC>
  Dir Element_City_Intersection<EC>::FindBestRouteStandard(EventWindow<EC>& window,
                                                           u32 destinationType,
                                                           Dir comingFrom) const
  {

    u32 roadFitness[] = {100, 100, 100};
    SPoint roads[3];
    Dir dirs[3]; /* The non-canal directions */
    u32 i = 0;
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();
    for (Dir d = Dirs::NORTH; d < Dirs::DIR_COUNT; d += 2)
    {
      if(d != Dirs::OppositeDir(comingFrom))
      {
        dirs[i] = d;
        Dirs::FillDir(roads[i], dirs[i], isStaggered);
        i++;
      }
    }

    for(u32 i = 0; i < 3; i++)
    {
      if(window.IsLiveSiteDirect(roads[i]))
      {
        if(window.GetRelativeAtomDirect(roads[i]).GetType() != GetStreetType() &&
           window.GetRelativeAtomDirect(roads[i]).GetType() != GetCarType())
        {
          roadFitness[i] += 100; /* That isn't a road! */
        }
        else
        {
          for(u32 j = 0; j < 2; j++)
          {
            SPoint edgeSidewalk;
            if(j == 0)
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CWDir(Dirs::CWDir(dirs[i])), isStaggered);
            }
            else
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CCWDir(Dirs::CCWDir(dirs[i])), isStaggered);
            }

            edgeSidewalk = edgeSidewalk + roads[i];
            if(window.GetRelativeAtomDirect(edgeSidewalk).GetType() !=
               Element_City_Sidewalk<EC>::THE_INSTANCE.GetType())
            {
              roadFitness[i] += 100; /* Don't want to go that way! */
            }
            else
            {
              if(Element_City_Sidewalk<EC>::THE_INSTANCE.
                 IsReadyToBuild(window.GetRelativeAtomDirect(edgeSidewalk)))
              {
                roadFitness[i] -=
                3 - (
                Element_City_Sidewalk<EC>::THE_INSTANCE.
                GetBuildingCount(window.
                                 GetRelativeAtomDirect(edgeSidewalk), destinationType));
              }
              else
              {
                roadFitness[i]++;
              }
            }
          }
        }
      }
    }

    u32 bestDirValue = MIN(roadFitness[0],
                           MIN(roadFitness[1], roadFitness[2]));
    u32 bestCount = 0;
    Dir dirsOfMin[3];

    for(u32 i = 0; i < 3; i++)
    {
      if(roadFitness[i] == bestDirValue)
      {
        dirsOfMin[bestCount++] = dirs[i];
      }
    }

    Dir bestDir = dirsOfMin[window.GetRandom().Create(bestCount)];

    return bestDir;
  }

  template <class EC>
  void Element_City_Intersection<EC>::UTurnCar(EventWindow<EC>& window,
                                               SPoint& carAt) const
  {
    T uTurnCar = window.GetRelativeAtomDirect(carAt);
    Element_City_Car<EC>::THE_INSTANCE.
    SetDirection(uTurnCar,
                 Dirs::OppositeDir(
                   Element_City_Car<EC>::THE_INSTANCE.
                   GetDirection(uTurnCar)));
    window.SetRelativeAtomDirect(carAt, uTurnCar);
    return;
  }

  template <class EC>
  void Element_City_Intersection<EC>::DoRouting(EventWindow<EC>& window) const
  {
    WindowScanner<EC> scanner(window);
    SPoint carToMove;
    u32 streetsAndCars;
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();

    if(scanner.FindRandomInVonNeumann(GetCarType(), carToMove) > 0)
    {
      /* First, let's see if this is a car we should route. */
      if(Element_City_Car<EC>::THE_INSTANCE.GetDirection(window.GetRelativeAtomDirect(carToMove)) !=
	 Dirs::OppositeDir(Dirs::FromOffset(carToMove))) {
	/* We probably just put that car there. Let's let it go. */

	T newCar = window.GetRelativeAtomDirect(carToMove);
	Element_City_Car<EC>::THE_INSTANCE.SetDirection(newCar, Dirs::FromOffset(carToMove));
	window.SetRelativeAtomDirect(carToMove, newCar);
	return;
      }

      streetsAndCars =
      scanner.CountVonNeumannNeighbors(GetCarType()) +
      scanner.CountVonNeumannNeighbors(GetStreetType());
      /* If there's nowhere else to go, turn around. */
      if (streetsAndCars == 1)
      {
        UTurnCar(window, carToMove);
        return;
      }

      Dir bestRoute = Dirs::NORTH;
      SPoint bestDirPt;

      /* Stupid two-sidewalk corner cases */
      if (streetsAndCars == 2)
      {
        Dir start = Dirs::FromOffset(carToMove);
        SPoint pt;
        bool found = true;
        for(u32 i = 0; i < 3; i++)
        {
          start = Dirs::CWDir(Dirs::CWDir(start));
          Dirs::FillDir(pt, start, isStaggered);
          if ((window.GetRelativeAtomDirect(pt).GetType() == GetCarType()) ||
              (window.GetRelativeAtomDirect(pt).GetType() == GetStreetType()))
          {
            bestRoute = start;
            bestDirPt = pt;
            break;
          }
        }
        if (!found)
        {
          FAIL(ILLEGAL_STATE);
        }
        else if(window.GetRelativeAtomDirect(pt).GetType() == GetStreetType())
        {
          /* It's a street, but we need to be careful. Let's give the
           * car the same direction that the street has. */
          T movingCar = window.GetRelativeAtomDirect(carToMove);
          Dir moveFrom = Element_City_Car<EC>::THE_INSTANCE.GetDirection(movingCar);

          T street = window.GetRelativeAtomDirect(bestDirPt);
          Dir streetDir = Element_City_Street<EC>::THE_INSTANCE.GetDirection(street);
          Element_City_Street<EC>::THE_INSTANCE.SetDirection(street, moveFrom);
          if (streetDir == Dirs::OppositeDir(start))
          {
            streetDir = start; /* Don't point the car at the intersection again */
          }
          Element_City_Car<EC>::THE_INSTANCE.SetDirection(movingCar, streetDir);

          window.SetRelativeAtomDirect(bestDirPt, movingCar);
          window.SetRelativeAtomDirect(carToMove, street);
          return;
        }
        /* Otherwise, it's a car and the normal behavior is sufficient. */
      }
      else
      {

#ifdef RANDOM_ROUTING
        bestRoute = FindRandomRoute(window);
#elif defined CANAL_ROUTING
        bestRoute = FindBestRouteCanal(
          window,
          Element_City_Car<EC>::THE_INSTANCE.
          GetDestType(window.GetRelativeAtomDirect(carToMove)),
          Element_City_Car<EC>::THE_INSTANCE.
          GetDirection(window.GetRelativeAtomDirect(carToMove)));
#else
        bestRoute = FindBestRouteStandard(
          window,
          Element_City_Car<EC>::THE_INSTANCE.
          GetDestType(window.GetRelativeAtomDirect(carToMove)),
          Element_City_Car<EC>::THE_INSTANCE.
          GetDirection(window.GetRelativeAtomDirect(carToMove)));
#endif

      }
      Dirs::FillDir(bestDirPt, bestRoute, isStaggered);

      if(window.GetRelativeAtomDirect(bestDirPt).GetType() == GetCarType())
      {
        /* If it's us, just change direction and move on. */
        if(bestDirPt.Equals(carToMove))
        {
          UTurnCar(window, carToMove);
          return;
        }

        /* If it's a car we can just swap, but we need to redirect both cars. */

        T movingCar = window.GetRelativeAtomDirect(carToMove);
        Dir moveFrom = Element_City_Car<EC>::THE_INSTANCE.GetDirection(movingCar);
        Element_City_Car<EC>::THE_INSTANCE.SetDirection(movingCar, bestRoute);

        T stoppedCar = window.GetRelativeAtomDirect(bestDirPt);
        Element_City_Car<EC>::THE_INSTANCE.SetDirection(stoppedCar, moveFrom);


        window.SetRelativeAtomDirect(bestDirPt, movingCar);
        window.SetRelativeAtomDirect(carToMove, stoppedCar);
      }
      else if(window.GetRelativeAtomDirect(bestDirPt).GetType() == GetStreetType())
      {
        /* Gotta be a street. We need to reconstruct the street with its correct direction. */

        T movingCar = window.GetRelativeAtomDirect(carToMove);
        Dir moveFrom = Element_City_Car<EC>::THE_INSTANCE.GetDirection(movingCar);
        Element_City_Car<EC>::THE_INSTANCE.SetDirection(movingCar, bestRoute);

        T street = window.GetRelativeAtomDirect(bestDirPt);
        Element_City_Street<EC>::THE_INSTANCE.
                                 SetDirection(street, moveFrom);

        window.SetRelativeAtomDirect(bestDirPt, movingCar);
        window.SetRelativeAtomDirect(carToMove, street);
      }
    }
  }

  template <class EC>
  void Element_City_Intersection<EC>::CreateStreetFromEmpty(EventWindow<EC>& window, Dir d) const
  {
    SPoint pt;
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();
    Dirs::FillDir(pt, d, isStaggered);
    if(window.GetRelativeAtomDirect(pt).GetType() ==
       Element_Empty<EC>::THE_INSTANCE.GetType())
    {
      Element_City_Street<EC>& st = Element_City_Street<EC>::THE_INSTANCE;
      T newStreet = st.GetDefaultAtom();
      st.SetDirection(newStreet, d);
      window.SetRelativeAtomDirect(pt, newStreet);
    }
  }

  template <class EC>
  void Element_City_Intersection<EC>::CreateSidewalkFromEmpty(EventWindow<EC>& window, Dir d) const
  {
    SPoint pt;
    bool isStaggered = window.GetTile().IsTileGridLayoutStaggered();
    Dirs::FillDir(pt, d, isStaggered);
    if(window.GetRelativeAtomDirect(pt).GetType() ==
       Element_Empty<EC>::THE_INSTANCE.GetType())
    {
      T newSidewalk = Element_City_Sidewalk<EC>::THE_INSTANCE.GetDefaultAtom();
      window.SetRelativeAtomDirect(pt, newSidewalk);
    }
  }

  template <class EC>
  void Element_City_Intersection<EC>::InitializeIntersection(T& atom,
                                                             EventWindow<EC>& window) const
  {
    Random& rand = window.GetRandom();
    Dir dirs[4] = { Dirs::NORTH, Dirs::EAST, Dirs::SOUTH, Dirs::WEST };
    SPoint offset;

    /* Shuffle dirs */
    for(u32 i = 0; i < 28; i++)
    {
      u32 a = rand.Create(4);
      u32 b = rand.Create(4);

      if(a != b)
      {
        Dir tmp = dirs[a];
        dirs[a] = dirs[b];
        dirs[b] = tmp;
      }
    }

    for(s32 i = 0; i < m_minCreatedStreets.GetValue(); i++)
    {
      CreateStreetFromEmpty(window, dirs[i]);
    }

    for(Dir d = Dirs::NORTH; d < Dirs::DIR_COUNT; d = (Dir)(d + 1))
    {
      CreateSidewalkFromEmpty(window, d);
    }
  }
}
