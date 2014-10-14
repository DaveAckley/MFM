/* -*- C++ -*- */
#include "MDist.h"

namespace MFM
{

  template <class CC>
  SPoint EventWindow<CC>::MapToTileValid(const SPoint& offset) const
  {
    if (!InWindow(offset)) FAIL(ILLEGAL_ARGUMENT);
    return MapToTile(offset);
  }

  template <class CC>
  const bool EventWindow<CC>::FillIfType(const SPoint relative, const u32 type, const T& atom)
  {
    bool replaced = false;
    if(GetRelativeAtom(relative).GetType() == type)
    {
      SetRelativeAtom(relative, atom);
      replaced = true;
    }
    return replaced;
  }

  template <class CC>
  const bool EventWindow<CC>::FillIfNotType(const SPoint relative, const u32 type, const T& atom)
  {
    bool replaced = false;
    if(GetRelativeAtom(relative).GetType() != type)
    {
      SetRelativeAtom(relative, atom);
      replaced = true;
    }
    return replaced;
  }

  template <class CC>
  bool EventWindow<CC>::SetRelativeAtom(const SPoint& offset, const T & atom)
  {
    if (IsLiveSite(offset))
    {
      m_tile.PlaceAtom(atom, MapToTileValid(offset));
      return true;
    }
    return false;
  }

  template <class CC>
  const typename CC::ATOM_TYPE& EventWindow<CC>::GetRelativeAtom(const SPoint& offset) const
  {
    return *m_tile.GetAtom(MapToTileValid(offset));
  }

  template <class CC>
  const typename CC::ATOM_TYPE& EventWindow<CC>::GetRelativeAtom(const Dir mooreOffset) const
  {
    SPoint pt;
    Dirs::FillDir(pt, mooreOffset);
    return GetRelativeAtom(pt);
  }

  template <class CC>
  bool EventWindow<CC>::CanSeeAtomOfType(const u32 type, const u32 radius) const
  {
    const MDist<R>& md = MDist<R>::get();

    if(radius == 0 || radius > R)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(radius); i++)
    {
      if(GetRelativeAtom(md.GetPoint(i)).GetType() == type)
      {
        return true;
      }
    }
    return false;
  }

  template <class CC>
  bool EventWindow<CC>::IsBorderingMoore(const u32 type) const
  {
    return IsBorderingNeighborhood(type, MooreNeighborhood, 8);
  }

  template <class CC>
  bool EventWindow<CC>::IsBorderingVonNeumann(const u32 type) const
  {
    return IsBorderingNeighborhood(type, VonNeumannNeighborhood, 4);
  }

  template <class CC>
  bool EventWindow<CC>::IsBorderingNeighborhood(const u32 type,
                                                const Dir* dirs,
                                                const u32 dirCount) const
  {
    SPoint searchPt;
    for(u32 i = 0; i < dirCount; i++)
    {
      Dirs::FillDir(searchPt, dirs[i]);

      if(GetRelativeAtom(searchPt).GetType() == type)
      {
        return true;
      }
    }
    return false;
  }

  template <class CC>
  bool EventWindow<CC>::FindRandomLocationOfType(const u32 type, SPoint& outPoint) const
  {
    return FindRandomLocationOfType(type, R, outPoint);
  }

  template <class CC>
  bool EventWindow<CC>::FindRandomLocationOfType(const u32 type,
                                                 const u32 radius,
                                                 SPoint& outPoint) const
  {
    const MDist<R>& md = MDist<R>::get();
    u32 foundPts = 0;

    if(radius == 0 || radius > R)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

    for(u32 i = md.GetFirstIndex(1); i < md.GetLastIndex(radius); i++)
    {
      if(GetRelativeAtom(md.GetPoint(i)).GetType() == type)
      {
        foundPts++;
        if(GetRandom().OneIn(foundPts))
        {
          outPoint.Set(md.GetPoint(i));
        }
      }
    }

    return foundPts > 0;
  }

  template <class CC>
  bool EventWindow<CC>::FindRandomInMoore(const u32 type, SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(type, MooreNeighborhood,
                                    sizeof(MooreNeighborhood) / sizeof(Dir), outPoint);
  }

  template <class CC>
  bool EventWindow<CC>::FindRandomInVonNeumann(const u32 type, SPoint& outPoint) const
  {
    return FindRandomInNeighborhood(type, VonNeumannNeighborhood,
                                    sizeof(VonNeumannNeighborhood) / sizeof(Dir), outPoint);
  }


  template <class CC>
  bool EventWindow<CC>::FindRandomInNeighborhood(const u32 type, const Dir* dirs,
                                                 const u32 dirCount, SPoint& outPoint) const
  {
    SPoint searchPt;
    for(u32 i = 0; i < dirCount; i++)
    {
      Dirs::FillDir(searchPt, dirs[i]);

      if(GetRelativeAtom(searchPt).GetType() == type)
      {
        Dirs::FillDir(outPoint, dirs[i]);
        return true;
      }
    }
    return false;
  }

  template <class CC>
  void EventWindow<CC>::SwapAtoms(const SPoint& locA, const SPoint& locB)
  {
    SPoint arrLocA(MapToTileValid(locA));
    SPoint arrLocB(MapToTileValid(locB));

    T a = *m_tile.GetAtom(arrLocA);
    T b = *m_tile.GetAtom(arrLocB);
    m_tile.PlaceAtom(b, arrLocA);
    m_tile.PlaceAtom(a, arrLocB);
  }

  template <class CC>
  void EventWindow<CC>::SwapCenterAtom(const SPoint& relative)
  {
    SwapAtoms(SPoint(0, 0), relative);
  }


} /* namespace MFM */
