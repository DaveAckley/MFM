#ifndef GRID_H      /* -*- C++ -*- */
#define GRID_H

#include "itype.h"
#include "Tile.h"
#include "ElementTable.h"
#include "Random.h"

namespace MFM {

  template <class T,u32 R, u32 W, u32 H>
  class Grid
  {
  private:
    Random m_random;

    u32 m_seed;

    void ReinitSeed();

    const u32 m_width, m_height;

    SPoint m_lastEventTile;

    Tile<T,R> m_tiles[W][H];

#if 0 // Anybody using these?
    Tile<T,R>* GetTile(int position);

    inline u32 GetPosition(u32 x, u32 y)
    { return x + y * m_width; }
#endif

  public:
    Random& GetRandom() { return m_random; }

    friend class GridRenderer;

    void SetSeed(u32 seed);

    Grid();

    void Reinit();

    void Needed(const Element<T,R> & anElement) ;

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
     *
     * Note that although siteInGrid is specified in 'uncached'
     * coordinates (in which (0,0) is the upperleftmost uncached
     * location of the tile at (0,0)), siteInTile is returned in
     * 'including cache' coordinates (in which (R,R) is the
     * upperleftmost uncached location of the tile at (0,0).  See
     * MapGridToUncachedTile for an alternative.
     */
    bool MapGridToTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const;

    /**
     * Find the grid coordinate of the 'owning tile' (i.e., ignoring
     * caches) for the give siteInGrid.  Return false if there isn't
     * one, otherwise set tileInGrid and siteInTile appropriately and
     * return true.
     */
    bool MapGridToUncachedTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const;

    /**
     * Return the Grid height in Tiles
     */
    static u32 GetHeight() { return H; }

    /**
     * Return the Grid width in Tiles
     */
    static u32 GetWidth() { return W; }

    /**
     * Return the Grid height in (non-cache) sites
     */
    static u32 GetHeightSites() 
    {
      return GetHeight() * Tile<T,R>::OWNED_SIDE;
    }

    /**
     * Return the Grid width in (non-cache) sites
     */
    static u32 GetWidthSites() 
    {
      return GetWidth() * Tile<T,R>::OWNED_SIDE;
    }


    void Pause();

    void Unpause();

    void PlaceAtom(T& atom, const SPoint& location);

    const T* GetAtom(SPoint& location);

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

    /* Don't count caches! */
    static inline const u32 GetTotalSites()
    { return GetWidthSites() * GetHeightSites(); }

    u64 GetTotalEventsExecuted() const;

    /* Returns the maximum written value */
    void WriteEPSImage(FILE* outstrm) const;

    void ResetEPSCounts();

    u32 GetAtomCount(ElementType atomType) const;
  };
} /* namespace MFM */

#include "grid.tcc"

#endif /*GRID_H*/

