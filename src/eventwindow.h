#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "atom.h"
#include "point.h"

#define EVENT_WINDOW_RADIUS 2

class EventWindow
{
private:
  Atom** m_atoms;

public:

  EventWindow(Point<int> ctr, Atom** atoms);

  ~EventWindow();

  /* Finds the manhattan area of a 
   window of a particular radius. */
  static int ManhattanArea(int maxDistance);
  
};

#endif /*EVENTWINDOW_H*/
