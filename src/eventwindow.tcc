#include "eventwindow.h"
#include "manhattandir.h"

template <class T>
EventWindow<T>::EventWindow(Point<int>& center, T* atoms, u32 tileWidth)
{
  int numAtoms = ManhattanDir::ManhattanArea(EVENT_WINDOW_RADIUS);
  m_atoms = new T[numAtoms];
  m_atomCount = numAtoms;

  Point<int> current;
  for(int i = 0; i < numAtoms; i++)
  {
    ManhattanDir::FillFromBits(current, i, MANHATTAN_TABLE_EVENT);
    T atom = atoms[current.GetX() + current.GetY() * tileWidth];
    m_atoms[i] = atom;
  }
}

template <class T>
EventWindow<T>::~EventWindow()
{
  delete m_atoms;
}

template <class T>
T* EventWindow<T>::GetCenterAtom()
{
  return m_atoms + (m_atomCount >> 1) + 1;
}
