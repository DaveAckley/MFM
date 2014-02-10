#ifndef EVENTWINDOW_H      /* -*- C++ -*- */
#define EVENTWINDOW_H

#include "point.h"
#include "itype.h"
#include "manhattandir.h"  /* for EVENT_WINDOW_SITES */

namespace MFM {

template <class T,u32 R>
class Tile;


template <class T,u32 R>
class EventWindow
{
private:

  typedef u32 (* StateFunction )(T* atom);

  Tile<T,R> & m_tile;

  SPoint m_center;

public:
  Random & GetRandom() { return m_tile.GetRandom(); }

  EventWindow(Tile<T,R> & tile, u32 tileWidth, u8 neighborConnections);

  EventWindow(Tile<T,R> & tile) : m_tile(tile) { }

  void SetCenter(const SPoint& center) ;

  ~EventWindow() { }

  u32 GetAtomCount()
  {
    return EVENT_WINDOW_SITES(R);
  }

  T& GetCenterAtom();

  T& GetRelativeAtom(const SPoint& offset);

  bool SetRelativeAtom(const SPoint& offset, T atom);

  void SwapAtoms(const SPoint& locA, const SPoint& locB);

  void FillCenter(SPoint& out);
  
};
} /* namespace MFM */

#include "eventwindow.tcc"

#endif /*EVENTWINDOW_H*/

