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
	foundPoints++;
	if(GetRandom().OneIn(foundPoints))
	{
	  outPoint.Set(md.GetPoint(i));
	}
      }
    }

    return foundPoints > 0;
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


} /* namespace MFM */
