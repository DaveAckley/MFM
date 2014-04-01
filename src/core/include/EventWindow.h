#ifndef EVENTWINDOW_H      /* -*- C++ -*- */
#define EVENTWINDOW_H

#include "Point.h"
#include "itype.h"
#include "MDist.h"  /* for EVENT_WINDOW_SITES */

namespace MFM {

  template <class CC>
  class Tile;


  template <class CC>
  class EventWindow
  {
  private:
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { W = P::TILE_WIDTH };
    enum { B = P::ELEMENT_TABLE_BITS };

    Tile<CC> & m_tile;

    SPoint m_center;

  public:
    Random & GetRandom() { return m_tile.GetRandom(); }

    Tile<CC>& GetTile() { return m_tile; }

    bool IsLiveSite(const SPoint & location) const {
      return m_tile.IsLiveSite(location+m_center);
    }

    EventWindow(Tile<CC> & tile, u32 tileWidth, u8 neighborConnections);

    EventWindow(Tile<CC> & tile) : m_tile(tile) { }

    void SetCenter(const SPoint& center) ;

    const SPoint& GetCenter() const { return m_center; }

    ~EventWindow() { }

    u32 GetAtomCount()
    {
      return EVENT_WINDOW_SITES(R);
    }

    const T& GetCenterAtom() const
    {
      return *m_tile.GetAtom(m_center);
    }

    void SetCenterAtom(const T& atom) 
    {
      return m_tile.PlaceAtom(atom,m_center);
    }

    const T& GetRelativeAtom(const SPoint& offset) const;

    bool SetRelativeAtom(const SPoint& offset, const T & atom);

    void SwapAtoms(const SPoint& locA, const SPoint& locB);

    void FillCenter(SPoint& out) const;
  
  };
} /* namespace MFM */

#include "EventWindow.tcc"

#endif /*EVENTWINDOW_H*/

