#ifndef EVENTWINDOW_H      /* -*- C++ -*- */
#define EVENTWINDOW_H

#include "point.h"
#include "itype.h"

template <class T,u32 R>
class Tile;

template <class T,u32 R>
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

  ~EventWindow() { }

  u32 GetAtomCount()
  {
    return m_atomCount;
  }

  T& GetCenterAtom();

  T& GetRelativeAtom(Point<int>& offset);

  void SetRelativeAtom(Point<int>& offset, T atom);

  void SwapAtoms(Point<int>& locA, Point<int>& locB);

  void FillCenter(Point<int>& out);
  
};

#include "eventwindow.tcc"

#endif /*EVENTWINDOW_H*/
