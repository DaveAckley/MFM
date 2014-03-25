/* -*- C++ -*- */
#include "MDist.h"

namespace MFM {

template <class C>
EventWindow<C>::EventWindow(Tile<C> & tile, u32 tileWidth,
			      u8 neighborConnections) : m_tile(tile)
{
  // m_tileWidth = tileWidth;
  //  m_neighborConnections = neighborConnections;

}

template <class C>
void EventWindow<C>::SetCenter(const SPoint& center)
{
  m_center = center;
}

template <class C> 
bool EventWindow<C>::SetRelativeAtom(const SPoint& offset, T atom)
{
  s32 idx = MDist<R>::get().FromPoint(offset, R);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  SPoint accessLoc(offset);
  accessLoc.Add(m_center.GetX(), m_center.GetY());
  
  m_tile.PlaceAtom(atom, accessLoc);

  return true;
}

template <class C>
const typename C::ATOM_TYPE& EventWindow<C>::GetRelativeAtom(const SPoint& offset) const
{
  s32 idx = MDist<R>::get().FromPoint(offset, R);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  SPoint accessLoc(offset);
  accessLoc.Add(m_center.GetX(), m_center.GetY());

  return *m_tile.GetAtom(accessLoc);

}

template <class C>
void EventWindow<C>::SwapAtoms(const SPoint& locA, const SPoint& locB)
{
  s32 aIdx = MDist<R>::get().FromPoint(locA, R);
  s32 bIdx = MDist<R>::get().FromPoint(locB, R);

  if (aIdx < 0) FAIL(ILLEGAL_ARGUMENT);
  if (bIdx < 0) FAIL(ILLEGAL_ARGUMENT);

  SPoint arrLocA(locA);
  SPoint arrLocB(locB);

  arrLocA.Add(m_center.GetX(), m_center.GetY());
  arrLocB.Add(m_center.GetX(), m_center.GetY());

  T a = *m_tile.GetAtom(arrLocA);
  T b = *m_tile.GetAtom(arrLocB);
  m_tile.PlaceAtom(b, arrLocA);
  m_tile.PlaceAtom(a, arrLocB);
}


template <class C>
void EventWindow<C>::FillCenter(SPoint& out) const
{
  out.Set(m_center.GetX(), m_center.GetY());
}
} /* namespace MFM */
