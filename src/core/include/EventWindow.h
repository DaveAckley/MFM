#ifndef EVENTWINDOW_H      /* -*- C++ -*- */
#define EVENTWINDOW_H

#include "Point.h"
#include "itype.h"
#include "MDist.h"  /* for EVENT_WINDOW_SITES */
#include "PSym.h"   /* For PointSymmetry, Map */

namespace MFM {

  // Forward declaration
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

    PointSymmetry m_sym;

    /**
     * Low-level, private because this does not guarantee loc is in
     * the window!
     */
    SPoint MapToTile(const SPoint & loc) const
    {
      return Map(loc,m_sym,loc)+m_center;
    }

  public:

    bool InWindow(const SPoint & offset) const
    {
      // Ignores m_sym since point symmetries can't change this answer
      return offset.GetManhattanLength() <= R;
    }

    /**
     * FAIL(ILLEGAL_ARGUMENT) if offset is not in the event window
     */
    SPoint MapToTileValid(const SPoint & offset) const ;

    PointSymmetry GetSymmetry() const { return m_sym; }

    void SetSymmetry(const PointSymmetry psym) {
      m_sym = psym;
    }

    Random & GetRandom() { return m_tile.GetRandom(); }

    Tile<CC>& GetTile() { return m_tile; }

    bool IsLiveSite(const SPoint & location) const {
      return m_tile.IsLiveSite(MapToTile(location));
    }

    EventWindow(Tile<CC> & tile) : m_tile(tile), m_sym(PSYM_NORMAL) { }

    /**
     * Place this EventWindow within GetTile, in untransformed Tile
     * coordinates.
     */
    void SetCenterInTile(const SPoint& center) {
      m_center = center;
    }

    /**
     * Get the position this EventWindow within GetTile, in
     * untransformed Tile coordinates.
     */
    const SPoint& GetCenterInTile() const { return m_center; }

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
      return m_tile.PlaceAtom(atom, m_center);
    }

    const T& GetRelativeAtom(const SPoint& offset) const;

    bool SetRelativeAtom(const SPoint& offset, const T & atom);

    void SwapAtoms(const SPoint& locA, const SPoint& locB);

    // Deprecated: Use GetCenterInTile instead
    // void FillCenter(SPoint& out) const;

  };
} /* namespace MFM */

#include "EventWindow.tcc"

#endif /*EVENTWINDOW_H*/

