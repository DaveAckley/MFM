#include "eventwindow.h"

template <class T>
EventWindow::EventWindow(T* atoms)
{
  m_atoms = atoms;
}

template <class T>
int EventWindow::ManhattanArea(int maxDistance)
{
  int oddSum = 0;
  int oddAcc = 1;
  for(int i = 0; i < maxDistance; i++)
  {
    oddSum += oddAcc;
    oddAcc += 2;
  }
  return (oddSum << 1) + oddAcc;
}
