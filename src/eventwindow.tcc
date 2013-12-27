#include "eventwindow.h"
#include "manhattandir.h"

template <class T>
EventWindow<T>::EventWindow(Point<int>& center, T* atoms, u32 tileWidth)
{
  int numAtoms = ManhattanDir::GetTableSize(MANHATTAN_TABLE_EVENT);
  m_atoms = new T[numAtoms];
  m_atomCount = numAtoms;

  Point<int> current;
  for(int i = 0; i < numAtoms; i++)
  {
    ManhattanDir::FillFromBits(current, i, MANHATTAN_TABLE_EVENT);
    T atom = atoms[current.GetX() + current.GetY() * tileWidth];
    m_atoms[i] = atom;
  }

  m_center.Set(center.GetX(), center.GetY());
}

template <class T>
EventWindow<T>::~EventWindow()
{
  delete[] m_atoms;
}

template <class T>
T* EventWindow<T>::GetCenterAtom()
{
  return m_atoms + (m_atomCount >> 1) + 1;
}

template <class T>
void EventWindow<T>::FillCenter(Point<int>& out)
{
  out.Set(m_center.GetX(), m_center.GetY());
}
