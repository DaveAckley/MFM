#ifndef GRID_H      /* -*- C++ -*- */
#define GRID_H

#include "itype.h"
#include "Tile.h"
#include "elementtable.h"
#include "random.h"

namespace MFM {

  template <class T,u32 R, u32 W, u32 H>
  class Grid
  {
  private:
    Random m_random;
    Random& GetRandom() { return m_random; }

    const u32 m_width, m_height;

    SPoint m_lastEventTile;

    Tile<T,R> m_tiles[W][H];

    Tile<T,R>* GetTile(int position);

    inline u32 GetPosition(u32 x, u32 y)
    { return x + y * m_width; }

  public:

    friend class GridRenderer;

    Grid();

    void Needed(const Element<T,R> & anElement) ;

    void SetSeed(u32 seed);

    ~Grid();

    /**
     * Return true iff tileInGrid is a legal tile coordinate in this
     * grid.  If this returns false, GetTile(tileInGrid) is unsafe.
     */
    bool IsLegalTileIndex(const SPoint & tileInGrid) const;

    /**
     * Find the grid coordinate of the 'owning tile' (i.e., ignoring
     * caches) for the give siteInGrid.  Return false if there isn't
     * one, otherwise set tileInGrid and siteInTile appropriately and
     * return true.
     */
    bool MapGridToTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const;

    u32 GetHeight();

    u32 GetWidth();

    void Pause();

    void Unpause();

    void PlaceAtom(T& atom, const SPoint& location);

    T* GetAtom(SPoint& location);

    void FillLastEventTile(SPoint& out);

    void TriggerEvent();

    inline Tile<T,R> & GetTile(const SPoint& pt)
    {return GetTile(pt.GetX(), pt.GetY());}

    inline const Tile<T,R> & GetTile(const SPoint& pt) const
    {return GetTile(pt.GetX(), pt.GetY());}

    inline Tile<T,R> & GetTile(u32 x, u32 y)
    { return m_tiles[x][y]; }

    inline const Tile<T,R> & GetTile(u32 x, u32 y) const
    { return m_tiles[x][y]; }

    u32 GetAtomCount(ElementType atomType);
  };
} /* namespace MFM */

#include "grid.tcc"

#endif /*GRID_H*/

