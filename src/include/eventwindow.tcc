/* -*- C++ -*- */
#include "manhattandir.h"

template <class T,u32 R>
EventWindow<T,R>::EventWindow(Point<int>& center, T* atoms, u32 tileWidth)
{
  int numAtoms = ManhattanDir<R>::get().GetTableSize(MANHATTAN_TABLE_EVENT);
  m_atoms = new T[numAtoms];
  m_atomCount = numAtoms;
  m_tileWidth = tileWidth;

  Point<int> current;
  for(int i = 0; i < numAtoms; i++)
  {
    ManhattanDir<R>::get().FillFromBits(current, i, MANHATTAN_TABLE_EVENT);
    current.Add(center);
    T atom = atoms[current.GetX() + current.GetY() * tileWidth];
    m_atoms[i] = atom;
  }

  m_center.Set(center.GetX(), center.GetY());
}

template <class T, u32 R>
void EventWindow<T,R>::DeallocateAtoms()
{
  delete[] m_atoms;
}

template <class T, u32 R>
T& EventWindow<T,R>::GetCenterAtom()
{
  return m_atoms[0];
}

template <class T, u32 R>
void EventWindow<T,R>::SetRelativeAtom(Point<int>& offset, T atom)
{
  s32 idx = ManhattanDir<R>::get().FromPoint(offset, MANHATTAN_TABLE_EVENT);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  m_atoms[idx] = atom;
}

template <class T, u32 R>
T& EventWindow<T,R>::GetRelativeAtom(Point<int>& offset)
{
  s32 idx = ManhattanDir<R>::get().FromPoint(offset, MANHATTAN_TABLE_EVENT);
  if (idx < 0)
    FAIL(ILLEGAL_ARGUMENT);

  return m_atoms[idx];
}

template <class T, u32 R>
void EventWindow<T,R>::SwapAtoms(Point<int>& locA, Point<int>& locB)
{
  s32 aIdx = ManhattanDir<R>::get().FromPoint(locA, MANHATTAN_TABLE_EVENT);
  s32 bIdx = ManhattanDir<R>::get().FromPoint(locB, MANHATTAN_TABLE_EVENT);

  if (aIdx < 0) FAIL(ILLEGAL_ARGUMENT);
  if (bIdx < 0) FAIL(ILLEGAL_ARGUMENT);

  T atom = m_atoms[aIdx];
  m_atoms[aIdx] = m_atoms[bIdx];
  m_atoms[bIdx] = atom;
}

template <class T, u32 R>
void EventWindow<T,R>::FillCenter(Point<int>& out)
{
  out.Set(m_center.GetX(), m_center.GetY());
}

template <class T, u32 R>
void EventWindow<T,R>::WriteTo(T* atoms, u16 tileWidth)
{
  Point<int> coffset;
  for(u32 i = 0; i < ManhattanDir<R>::get().GetTableSize(MANHATTAN_TABLE_EVENT); i++)
  {
    ManhattanDir<R>::get().FillFromBits(coffset, i, MANHATTAN_TABLE_EVENT);
    coffset.Add(m_center);

    if(coffset.GetX() >= 0 && coffset.GetY() >= 0 &&
       coffset.GetX() < tileWidth && coffset.GetY() < tileWidth)
    {
      Point<int> relative(coffset.GetX() - m_center.GetX(),
			  coffset.GetY() - m_center.GetY());
      
      atoms[coffset.GetX() + coffset.GetY() * tileWidth] =
	GetRelativeAtom(relative);
    }
  }
}
