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
  const typename CC::ATOM_TYPE& EventWindow<CC>::GetRelativeAtom(const Dir mooreOffset) const
  {
    SPoint pt;
    Dirs::FillDir(pt, mooreOffset);
    return GetRelativeAtom(pt);
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
