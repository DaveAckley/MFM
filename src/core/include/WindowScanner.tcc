/* -*- C++ -*- */

#include "Element_Empty.h"

#include <stdarg.h>

namespace MFM
{
  template <class EC>
  WindowScanner<EC>::WindowScanner(EventWindow<EC>& window) :
    m_win(window),
    m_rand(window.GetRandom()),
    EMPTY_TYPE(Element_Empty<EC>::THE_INSTANCE.GetType())
  { }

  template <class EC>
  bool WindowScanner<EC>::FillIfType(const SPoint& relative, const u32 type, const T& atom)
  {
    bool replaced = false;
    if(m_win.IsLiveSite(relative))
    {
      if(m_win.GetRelativeAtom(relative).GetType() == type)
      {
        m_win.SetRelativeAtom(relative, atom);
        replaced = true;
      }
    }
    return replaced;
  }

  template <class EC>
  bool WindowScanner<EC>::FillIfNotType(const SPoint& relative,
                                        const u32 type, const T& atom)
  {
    bool replaced = false;

    if(m_win.IsLiveSite(relative))
    {
      if(m_win.GetRelativeAtom(relative).GetType() != type)
      {
        m_win.SetRelativeAtom(relative, atom);
        replaced = true;
      }
    }
    return replaced;
  }

  template <class EC>
  bool WindowScanner<EC>::FillIfEmpty(const SPoint& relative, const T& atom)
  {
    return m_win.FillIfType(relative, EMPTY_TYPE, atom);
  }

  template <class EC>
  bool WindowScanner<EC>::CanSeeAtomOfType(const u32 type, const u32 radius) const
  {
    const MDist<R>& md = MDist<R>::get();

    MFM_API_ASSERT_ARG(radius != 0 && radius <= R);

    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(radius); i++)
    {
      if(m_win.IsLiveSite(md.GetPoint(i)))
      {
        if(m_win.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
        {
          return true;
        }
      }
    }
    return false;
  }

  template <class EC>
  u32 WindowScanner<EC>::CountAtomsOfType(const u32 type, const u32 radius) const
  {
    const MDist<R>& md = MDist<R>::get();
    u32 atomCount = 0;

    MFM_API_ASSERT_ARG(radius != 0 && radius <= R);

    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(radius); i++)
    {
      if(m_win.IsLiveSite(md.GetPoint(i)))
      {
        if(m_win.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
        {
          atomCount++;
        }
      }
    }
    return atomCount;
  }

  template <class EC>
  u32 WindowScanner<EC>::CountEmptyAtoms(const u32 radius) const
  {
    return CountAtomsOfType(EMPTY_TYPE, radius);
  }

  template <class EC>
  bool WindowScanner<EC>::IsBorderingNeighborhood(const u32 type,
                                                  const Dir* neighborhood,
                                                  const u32 dirCount) const
  {
    SPoint searchPt;
    for(u32 i = 0; i < dirCount; i++)
    {
      Dirs::FillDir(searchPt, neighborhood[i]);
      if(m_win.IsLiveSite(searchPt))
      {
        if(m_win.GetRelativeAtom(searchPt).GetType() == type)
        {
          return true;
        }
      }
    }
    return false;
  }

  template <class EC>
  bool WindowScanner<EC>::IsBorderingMoore(const u32 type) const
  {
    return IsBorderingNeighborhood(type,
                                   MooreNeighborhood,
                                   MOORE_NEIGHBORHOOD_SIZE);
  }

  template <class EC>
  bool WindowScanner<EC>::IsBorderingVonNeumann(const u32 type) const
  {
    return IsBorderingNeighborhood(type,
                                   VonNeumannNeighborhood,
                                   VON_NEUMANN_NEIGHBORHOOD_SIZE);
  }

  template <class EC>
  u32 WindowScanner<EC>::CountInNeighborhood(const u32 type,
                                           const Dir* neighborhood,
                                           const u32 dirCount) const
  {
    SPoint searchPt;
    u32 atomCount = 0;
    for(u32 i = 0; i < dirCount; i++)
    {
      Dirs::FillDir(searchPt, neighborhood[i]);

      if(m_win.IsLiveSite(searchPt))
      {
        if(m_win.GetRelativeAtom(searchPt).GetType() == type)
        {
          atomCount++;
        }
      }
    }
    return atomCount;
  }

  template <class EC>
  u32 WindowScanner<EC>::CountMooreNeighbors(const u32 type) const
  {
    return CountInNeighborhood(type,
                               MooreNeighborhood,
                               MOORE_NEIGHBORHOOD_SIZE);
  }

  template <class EC>
  u32 WindowScanner<EC>::CountVonNeumannNeighbors(const u32 type) const
  {
    return CountInNeighborhood(type,
                               VonNeumannNeighborhood,
                               VON_NEUMANN_NEIGHBORHOOD_SIZE);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindRandomLocationOfType(const u32 type, SPoint& outPoint) const
  {
    return FindRandomLocationOfType(type, R, outPoint);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindRandomLocationOfType(const u32 type,
                                                  const u32 radius,
                                                  SPoint& outPoint) const
  {
    const MDist<R>& md = MDist<R>::get();
    u32 foundPts = 0;

    MFM_API_ASSERT_ARG(radius != 0 && radius <= R);

    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(radius); i++)
    {
      if(m_win.IsLiveSite(md.GetPoint(i)))
      {
        if(m_win.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
        {
          foundPts++;
          if(m_rand.OneIn(foundPts))
          {
            outPoint.Set(md.GetPoint(i));
          }
        }
      }
    }
  return foundPts;
  }

  template <class EC>
  u32 WindowScanner<EC>::FindRandomInSubWindow(const u32 type, const SPoint* subWindow,
                                               const u32 subCount, SPoint& outPoint) const
  {
    u32 atomCount = 0;
    for(u32 i = 0; i < subCount; i++)
    {
      if(m_win.IsLiveSiteSym(subWindow[i]))
      {
        if(m_win.GetRelativeAtomSym(subWindow[i]).GetType() == type)
        {
          atomCount++;
          if(m_rand.OneIn(atomCount))
          {
            outPoint.Set(subWindow[i].GetX(), subWindow[i].GetY());
          }
        }
      }
    }
    return atomCount;
  }

  template <class EC>
  u32 WindowScanner<EC>::FindEmptyInSubWindow(const SPoint* subWindow,
                                              const u32 subCount, SPoint& outPoint) const
  {
    return FindRandomInSubWindow(EMPTY_TYPE, subWindow, subCount, outPoint);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindRandomInNeighborhood(const u32 type,
                                                  const Dir* dirs,
                                                  const u32 dirCount,
                                                  SPoint& outPoint) const
  {
    SPoint searchPt;
    u32 ptsFound = 0;
    for(u32 i = 0; i < dirCount; i++)
    {
      Dirs::FillDir(searchPt, dirs[i]);

      if(m_win.IsLiveSiteSym(searchPt))
      {
        if(m_win.GetRelativeAtomSym(searchPt).GetType() == type)
        {
          ptsFound++;
          if(m_rand.OneIn(ptsFound))
          {
            Dirs::FillDir(outPoint, dirs[i]);
          }
        }
      }
    }
    return ptsFound;
  }

  template <class EC>
  u32 WindowScanner<EC>::FindEmptyInNeighborhood(const Dir* dirs,
                                                 const u32 dirCount,
                                                 SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(EMPTY_TYPE,
                                    dirs,
                                    dirCount,
                                    outPoint);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindRandomInMoore(const u32 type, SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(type,
                                    MooreNeighborhood,
                                    MOORE_NEIGHBORHOOD_SIZE,
                                    outPoint);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindRandomInVonNeumann(const u32 type, SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(type,
                                    VonNeumannNeighborhood,
                                    VON_NEUMANN_NEIGHBORHOOD_SIZE,
                                    outPoint);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindEmptyInMoore(SPoint& outPoint) const
  {
    return FindEmptyInNeighborhood(MooreNeighborhood,
                                   MOORE_NEIGHBORHOOD_SIZE,
                                   outPoint);
  }

  template <class EC>
  u32 WindowScanner<EC>::FindEmptyInVonNeumann(SPoint& outPoint) const
  {
    return FindEmptyInNeighborhood(VonNeumannNeighborhood,
                                   VON_NEUMANN_NEIGHBORHOOD_SIZE,
                                   outPoint);
  }

  template <class EC>
  void WindowScanner<EC>::FindRandomAtoms(const u32 radius, const u32 count, ...) const
  {
    va_list argList;
    va_start(argList, count);
    {
      FindRandomAtoms(radius, count, argList);
    }
    va_end(argList);
  }

  template <class EC>
  void WindowScanner<EC>::FindRandomAtoms(const u32 count, ...) const
  {
    va_list argList;
    va_start(argList, count);
    {
      FindRandomAtoms(R, count, argList);
    }
    va_end(argList);
  }

  template <class EC>
  bool WindowScanner<EC>::FindRandomAtomsInSubWindows(const u32 count, ...) const
  {
    va_list argList;
    bool foundAll = true;
    va_start(argList, count);
    {
      for(u32 i = 0; i < count; i++)
      {
        SPoint& outPt = *((SPoint*)va_arg(argList, SPoint*));
        u32 type = (u32)va_arg(argList, u32);
        SPoint* dirs = (SPoint*)va_arg(argList, SPoint*);
        u32 dirCount = (u32)va_arg(argList, u32);

        if(FindRandomInSubWindow(type, dirs, dirCount, outPt) == 0)
        {
          foundAll = false;
        }
      }
    }
    va_end(argList);

    return foundAll;
  }

  template <class EC>
  void WindowScanner<EC>::FindRandomAtoms(const u32 radius, const u32 count, va_list& list) const
  {
    /* Can't ask for more than SITES things, right? */
    MFM_API_ASSERT_ARG(count <= SITES);

    const MDist<R>& md = MDist<R>::get();
    Random& rand = m_win.GetRandom();
    SPoint* outPts[SITES];
    u32 types[SITES];
    u32* outCounts[SITES];

    for(u32 i = 0; i < count; i++)
    {
      outPts[i] = (SPoint*)va_arg(list, SPoint*);
      types[i] = (u32)va_arg(list, u32);
      outCounts[i] = (u32*)va_arg(list, u32*);

      *outCounts[i] = 0;
    }

    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(radius); i++)
    {
      SPoint pt = md.GetPoint(i);
      if(m_win.IsLiveSiteSym(pt))
      {
        for(u32 j = 0; j < count; j++)
        {
          if(m_win.GetRelativeAtomSym(pt).GetType() == types[j])
          {
            (*(outCounts[j]))++;
            if(rand.OneIn(*(outCounts[j])))
            {
              outPts[j]->Set(pt.GetX(), pt.GetY());
            }
          }
        }
      }
    }
  }
}
