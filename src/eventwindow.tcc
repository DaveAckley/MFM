#include "eventwindow.h"
#include "manhattandir.h"

template <class T>
EventWindow<T>::EventWindow(Point<int>& center, T* atoms, u32 tileWidth)
{
  int numAtoms = ManhattanDir::GetTableSize(MANHATTAN_TABLE_EVENT);
  m_atoms = new T[numAtoms];
  m_atomCount = numAtoms;
  m_tileWidth = tileWidth;

  Point<int> current;
  for(int i = 0; i < numAtoms; i++)
  {
    ManhattanDir::FillFromBits(current, i, MANHATTAN_TABLE_EVENT);
    current.Add(center);
    T atom = atoms[current.GetX() + current.GetY() * tileWidth];
    m_atoms[i] = atom;
  }

  m_center.Set(center.GetX(), center.GetY());
}

template <class T>
void EventWindow<T>::DeallocateAtoms()
{
  delete[] m_atoms;
}

template <class T>
T& EventWindow<T>::GetCenterAtom()
{
  return m_atoms[m_atomCount >> 1];
}

template <class T>
T& EventWindow<T>::GetRelativeAtom(Point<int>& offset)
{
  u8 idx = ManhattanDir::FromPoint(offset, MANHATTAN_TABLE_EVENT);

  return m_atoms[idx];
}

template <class T>
void EventWindow<T>::SwapAtoms(Point<int>& locA, Point<int>& locB)
{
  u8 aIdx = ManhattanDir::FromPoint(locA, MANHATTAN_TABLE_EVENT);
  u8 bIdx = ManhattanDir::FromPoint(locB, MANHATTAN_TABLE_EVENT);

  T atom = m_atoms[aIdx];
  m_atoms[aIdx] = m_atoms[bIdx];
  m_atoms[bIdx] = atom;
}

template <class T>
void EventWindow<T>::FillCenter(Point<int>& out)
{
  out.Set(m_center.GetX(), m_center.GetY());
}

template <class T>
void EventWindow<T>::WriteTo(T* atoms, u16 tileWidth)
{
  Point<int> coffset;
  for(int i = 0; i < ManhattanDir::GetTableSize(MANHATTAN_TABLE_EVENT); i++)
  {
    ManhattanDir::FillFromBits(coffset, i, MANHATTAN_TABLE_EVENT);
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
