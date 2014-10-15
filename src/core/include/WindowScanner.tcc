/* -*- C++ -*- */

#include "Element_Empty.h"

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
    if(m_win.GetRelativeAtom(relative).GetType() == type)
    {
      m_win.SetRelativeAtom(relative, atom);
      replaced = true;
    }
    return replaced;
  }

  template <class CC>
  bool WindowScanner<CC>::FillIfNotType(const SPoint& relative,
                                        const u32 type, const T& atom)
  {
    bool replaced = false;
    if(m_win.GetRelativeAtom(relative).GetType() != type)
    {
      m_win.SetRelativeAtom(relative, atom);
      replaced = true;
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
      if(m_win.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
      {
        return true;
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
      if(m_win.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
      {
        atomCount++;
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
      if(m_win.GetRelativeAtom(searchPt).GetType() == type)
      {
        return true;
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
      if(m_win.GetRelativeAtom(searchPt).GetType() == type)
      {
        atomCount++;
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
      if(m_win.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
      {
        foundPts++;
        if(m_rand.OneIn(foundPts))
        {
          outPoint.Set(md.GetPoint(i));
        }
      }
    }
  return foundPts;
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

      if(m_win.GetRelativeAtom(searchPt).GetType() == type)
      {
        ptsFound++;
        if(m_rand.OneIn(ptsFound))
        {
          Dirs::FillDir(outPoint, dirs[i]);
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
}
