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

  u32 m_tileWidth;

  T* m_atoms;

  Point<int> m_center;

public:

  EventWindow(Point<int>& center, T* atoms, u32 tileWidth);

  EventWindow() { }

  void DeallocateAtoms();

  ~EventWindow() { }

  T& GetCenterAtom();

  T& GetRelativeAtom(Point<int>& offset);

  void SetRelativeAtom(Point<int>& offset, T atom);

  void SwapAtoms(Point<int>& locA, Point<int>& locB);

  void FillCenter(Point<int>& out);

  void WriteTo(T* atoms, u16 tileWidth);
  
};

#include "eventwindow.tcc"

#endif /*EVENTWINDOW_H*/
