#include "eventwindow.h"

EventWindow::EventWindow
(Point<int> center, Atom** atoms)
{
  int size = ManhattanArea(EVENT_WINDOW_RADIUS);
  m_atoms = 
    new Atom*[size];

  /* TODO fill with all atoms found in memory. */
}

EventWindow::~EventWindow()
{
  delete[] m_atoms;
}

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
