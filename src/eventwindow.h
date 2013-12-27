#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "point.h"
#include "itype.h"

#define EVENT_WINDOW_RADIUS 2

template <class T>
class Tile;

template <class T>
class EventWindow
{
private:
  u32 m_atomCount;

  T* m_atoms;

  Point<int> m_center;

public:

  EventWindow(Point<int>& center, T* atoms, u32 tileWidth);

  ~EventWindow();

  T* GetCenterAtom();

  void FillCenter(Point<int>& out);

  
  
};

#include "eventwindow.tcc"

#endif /*EVENTWINDOW_H*/
