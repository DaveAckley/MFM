/* -*- C++ -*- */
#include "manhattandir.h"

namespace MFM {

template <class T,u32 R>
EventWindow<T,R>::EventWindow(Tile<T,R> & tile, u32 tileWidth,
			      u8 neighborConnections) : m_tile(tile)
{
  // m_tileWidth = tileWidth;
  //  m_neighborConnections = neighborConnections;

}

template <class T, u32 R>
void EventWindow<T,R>::SetCenter(const SPoint& center)
{
  m_center = center;
}

template <class T, u32 R>
T& EventWindow<T,R>::GetCenterAtom()
{
  return *m_tile.GetAtom(m_center);
}

template <class T, u32 R>
bool EventWindow<T,R>::SetRelativeAtom(const SPoint& offset, T atom)
{
  s32 idx = ManhattanDir<R>::get().FromPoint(offset, (TableType)R);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  SPoint accessLoc(offset);
  accessLoc.Add(m_center.GetX(), m_center.GetY());
  
  m_tile.PlaceAtom(atom, accessLoc);

  return true;
}

template <class T, u32 R>
T& EventWindow<T,R>::GetRelativeAtom(const SPoint& offset)
{
  s32 idx = ManhattanDir<R>::get().FromPoint(offset, (TableType)R);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  SPoint accessLoc(offset);
  accessLoc.Add(m_center.GetX(), m_center.GetY());

  return *m_tile.GetAtom(accessLoc);

}

template <class T, u32 R>
void EventWindow<T,R>::SwapAtoms(const SPoint& locA, const SPoint& locB)
{
  s32 aIdx = ManhattanDir<R>::get().FromPoint(locA, (TableType)R);
  s32 bIdx = ManhattanDir<R>::get().FromPoint(locB, (TableType)R);

  if (aIdx < 0) FAIL(ILLEGAL_ARGUMENT);
  if (bIdx < 0) FAIL(ILLEGAL_ARGUMENT);

  SPoint arrLocA(locA);
  SPoint arrLocB(locB);

  arrLocA.Add(m_center.GetX(), m_center.GetY());
  arrLocB.Add(m_center.GetX(), m_center.GetY());

  T temp = *m_tile.GetAtom(arrLocA);
  *m_tile.GetAtom(arrLocA) = *m_tile.GetAtom(arrLocB);
  *m_tile.GetAtom(arrLocB) = temp;
}

template <class T, u32 R>
void EventWindow<T,R>::FillCenter(SPoint& out)
{
  out.Set(m_center.GetX(), m_center.GetY());
}
} /* namespace MFM */
