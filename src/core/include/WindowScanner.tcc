/* -*- C++ -*- */

#include "Element_Empty.h"

#include <stdarg.h>

namespace MFM
{
  template <class CC>
  WindowScanner<CC>::WindowScanner(EventWindow<CC>& window) :
    m_win(window),
    m_rand(window.GetRandom()),
    EMPTY_TYPE(Element_Empty<CC>::THE_INSTANCE.GetType())
  { }

  template <class CC>
  bool WindowScanner<CC>::FillIfType(const SPoint& relative, const u32 type, const T& atom)
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

  template <class CC>
  bool WindowScanner<CC>::FillIfNotType(const SPoint& relative,
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

  template <class CC>
  bool WindowScanner<CC>::FillIfEmpty(const SPoint& relative, const T& atom)
  {
    return m_win.FillIfType(relative, EMPTY_TYPE, atom);
  }

  template <class CC>
  bool WindowScanner<CC>::CanSeeAtomOfType(const u32 type, const u32 radius) const
  {
    const MDist<R>& md = MDist<R>::get();

    if(radius == 0 || radius > R)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

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

  template <class CC>
  u32 WindowScanner<CC>::CountAtomsOfType(const u32 type, const u32 radius) const
  {
    const MDist<R>& md = MDist<R>::get();
    u32 atomCount = 0;

    if(radius == 0 || radius > R)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

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

  template <class CC>
  u32 WindowScanner<CC>::CountEmptyAtoms(const u32 radius) const
  {
    return CountAtomsOfType(EMPTY_TYPE, radius);
  }

  template <class CC>
  bool WindowScanner<CC>::IsBorderingNeighborhood(const u32 type,
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

  template <class CC>
  bool WindowScanner<CC>::IsBorderingMoore(const u32 type) const
  {
    return IsBorderingNeighborhood(type,
                                   MooreNeighborhood,
                                   MOORE_NEIGHBORHOOD_SIZE);
  }

  template <class CC>
  bool WindowScanner<CC>::IsBorderingVonNeumann(const u32 type) const
  {
    return IsBorderingNeighborhood(type,
                                   VonNeumannNeighborhood,
                                   VON_NEUMANN_NEIGHBORHOOD_SIZE);
  }

  template <class CC>
  u32 WindowScanner<CC>::CountInNeighborhood(const u32 type,
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

  template <class CC>
  u32 WindowScanner<CC>::CountMooreNeighbors(const u32 type) const
  {
    return CountInNeighborhood(type,
                               MooreNeighborhood,
                               MOORE_NEIGHBORHOOD_SIZE);
  }

  template <class CC>
  u32 WindowScanner<CC>::CountVonNeumannNeighbors(const u32 type) const
  {
    return CountInNeighborhood(type,
                               VonNeumannNeighborhood,
                               VON_NEUMANN_NEIGHBORHOOD_SIZE);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindRandomLocationOfType(const u32 type, SPoint& outPoint) const
  {
    return FindRandomLocationOfType(type, R, outPoint);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindRandomLocationOfType(const u32 type,
                                                  const u32 radius,
                                                  SPoint& outPoint) const
  {
    const MDist<R>& md = MDist<R>::get();
    u32 foundPts = 0;

    if(radius == 0 || radius > R)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

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

  template <class CC>
  u32 WindowScanner<CC>::FindRandomInSubWindow(const u32 type, const SPoint* subWindow,
                                               const u32 subCount, SPoint& outPoint) const
  {
    u32 atomCount = 0;
    for(u32 i = 0; i < subCount; i++)
    {
      if(m_win.IsLiveSite(subWindow[i]))
      {
        if(m_win.GetRelativeAtom(subWindow[i]).GetType() == type)
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

  template <class CC>
  u32 WindowScanner<CC>::FindEmptyInSubWindow(const SPoint* subWindow,
                                              const u32 subCount, SPoint& outPoint) const
  {
    return FindRandomInSubWindow(EMPTY_TYPE, subWindow, subCount, outPoint);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindRandomInNeighborhood(const u32 type,
                                                  const Dir* dirs,
                                                  const u32 dirCount,
                                                  SPoint& outPoint) const
  {
    SPoint searchPt;
    u32 ptsFound = 0;
    for(u32 i = 0; i < dirCount; i++)
    {
      Dirs::FillDir(searchPt, dirs[i]);

      if(m_win.IsLiveSite(searchPt))
      {
        if(m_win.GetRelativeAtom(searchPt).GetType() == type)
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

  template <class CC>
  u32 WindowScanner<CC>::FindEmptyInNeighborhood(const Dir* dirs,
                                                 const u32 dirCount,
                                                 SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(EMPTY_TYPE,
                                    dirs,
                                    dirCount,
                                    outPoint);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindRandomInMoore(const u32 type, SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(type,
                                    MooreNeighborhood,
                                    MOORE_NEIGHBORHOOD_SIZE,
                                    outPoint);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindRandomInVonNeumann(const u32 type, SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(type,
                                    VonNeumannNeighborhood,
                                    VON_NEUMANN_NEIGHBORHOOD_SIZE,
                                    outPoint);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindEmptyInMoore(SPoint& outPoint) const
  {
    return FindEmptyInNeighborhood(MooreNeighborhood,
                                   MOORE_NEIGHBORHOOD_SIZE,
                                   outPoint);
  }

  template <class CC>
  u32 WindowScanner<CC>::FindEmptyInVonNeumann(SPoint& outPoint) const
  {
    return FindEmptyInNeighborhood(VonNeumannNeighborhood,
                                   VON_NEUMANN_NEIGHBORHOOD_SIZE,
                                   outPoint);
  }

  template <class CC>
  void WindowScanner<CC>::FindRandomAtoms(const u32 radius, const u32 count, ...) const
  {
    va_list argList;
    va_start(argList, count);
    {
      FindRandomAtoms(radius, count, argList);
    }
    va_end(argList);
  }

  template <class CC>
  void WindowScanner<CC>::FindRandomAtoms(const u32 count, ...) const
  {
    va_list argList;
    va_start(argList, count);
    {
      FindRandomAtoms(R, count, argList);
    }
    va_end(argList);
  }

  template <class CC>
  bool WindowScanner<CC>::FindRandomAtomsInSubWindows(const u32 count, ...) const
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

  template <class CC>
  void WindowScanner<CC>::FindRandomAtoms(const u32 radius, const u32 count, va_list& list) const
  {
    /* Can't ask for more than SITES things, right? */
    if(count > SITES)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

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
      if(m_win.IsLiveSite(pt))
      {
        for(u32 j = 0; j < count; j++)
        {
          if(m_win.GetRelativeAtom(pt).GetType() == types[j])
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
