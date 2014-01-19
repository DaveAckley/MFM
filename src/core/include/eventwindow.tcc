/* -*- C++ -*- */
#include "manhattandir.h"

template <class T,u32 R>
EventWindow<T,R>::EventWindow(Point<int>& center, T* atoms, u32 tileWidth,
			      u8 neighborConnections)
{
  m_atoms = atoms;
  m_atomCount = EVENT_WINDOW_SITES(R);
  m_tileWidth = tileWidth;
  
  m_neighborConnections = neighborConnections;

  m_center = Point<int>(center);
}

template <class T, u32 R>
T& EventWindow<T,R>::GetCenterAtom()
{
  return m_atoms[m_center.GetX() + m_center.GetY() * m_tileWidth];
}

template <class T, u32 R>
bool EventWindow<T,R>::SetRelativeAtom(Point<int>& offset, T atom)
{
  s32 idx = ManhattanDir<R>::get().FromPoint(offset, (TableType)R);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  Point<int> accessLoc(offset);
  accessLoc.Add(m_center.GetX(), m_center.GetY());

  m_atoms[accessLoc.GetX() + accessLoc.GetY() * m_tileWidth] = atom;

  return true;
}

template <class T, u32 R>
T& EventWindow<T,R>::GetRelativeAtom(Point<int>& offset)
{
  s32 idx = ManhattanDir<R>::get().FromPoint(offset, (TableType)R);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  Point<int> accessLoc(offset);
  accessLoc.Add(m_center.GetX(), m_center.GetY());

  return m_atoms[accessLoc.GetX() + accessLoc.GetY() * m_tileWidth];

}

template <class T, u32 R>
void EventWindow<T,R>::SwapAtoms(Point<int>& locA, Point<int>& locB)
{
  s32 aIdx = ManhattanDir<R>::get().FromPoint(locA, (TableType)R);
  s32 bIdx = ManhattanDir<R>::get().FromPoint(locB, (TableType)R);

  if (aIdx < 0) FAIL(ILLEGAL_ARGUMENT);
  if (bIdx < 0) FAIL(ILLEGAL_ARGUMENT);

  Point<int> arrLocA(locA);
  Point<int> arrLocB(locB);

  arrLocA.Add(m_center.GetX(), m_center.GetY());
  arrLocB.Add(m_center.GetX(), m_center.GetY());

  T atom = m_atoms[arrLocA.GetX() + arrLocA.GetY() * m_tileWidth];

  m_atoms[arrLocA.GetX() + arrLocA.GetY() * m_tileWidth] =
    m_atoms[arrLocB.GetX() + arrLocB.GetY() * m_tileWidth];

  m_atoms[arrLocB.GetX() + arrLocB.GetY() * m_tileWidth] = atom;
}

template <class T, u32 R>
void EventWindow<T,R>::FillCenter(Point<int>& out)
{
  out.Set(m_center.GetX(), m_center.GetY());
}
