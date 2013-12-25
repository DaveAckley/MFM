#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "point.h"

#define EVENT_WINDOW_RADIUS 2

template <class T>
class EventWindow
{
private:
  T* m_atoms;

public:

  EventWindow(T* atoms);

  ~EventWindow() { }

  

  /* Finds the manhattan area of a 
   window of a particular radius. */
  static int ManhattanArea(int maxDistance);
  
};

#endif /*EVENTWINDOW_H*/
