/*                                              -*- mode:C++ -*-
  Grid.h Encapsulator for all MFM logic
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Grid.h Encapsulator for all MFM logic
  \author Trent R. Small
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef GRID_H
#define GRID_H

#include "itype.h"
#include "Tile.h"
#include "ElementTable.h"
#include "Random.h"
#include "GridConfig.h"

#include "Element_Wall.h"

namespace MFM {

  template <class GC>
  class Grid
  {
    // Extract short type names
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};

  private:
    Random m_random;

    u32 m_seed;

    void ReinitSeed();

    const u32 m_width, m_height;

    SPoint m_lastEventTile;

    Tile<CC> m_tiles[W][H];

  public:
    Random& GetRandom() { return m_random; }

    friend class GridRenderer;

    void SetSeed(u32 seed);

    Grid() : m_seed(0), m_width(W), m_height(H)
    {
    }

    void Reinit();

    void Needed(const Element<CC> & anElement)
    {
      for(u32 i = 0; i < W; i++)
        for(u32 j = 0; j < H; j++)
          m_tiles[i][j].RegisterElement(anElement);
      LOG.Message("Assigned type 0x%04x for %@",anElement.GetType(),&anElement.GetUUID());
    }

    template <typename PointerType> class MyIterator {
      Grid & g;
      s32 i;
      s32 j;
    public:
      MyIterator(Grid<GC> & g, int i = 0, int j = 0) : g(g), i(i), j(j) { }

      bool operator!=(const MyIterator &m) const { return i != m.i || j != m.j; }
      void operator++() {
        if (j < g.H) {
          i++;
          if (i >= g.W) {
            i = 0;
            j++;
          }
        }
      }
      int operator-(const MyIterator &m) const {
        s32 rows = j-m.j;
        s32 cols = i-m.i;
        return rows*g.W + cols;
      }

      PointerType operator*() const {
        return &g.m_tiles[i][j];
      }

    };

    typedef MyIterator<Tile<CC>*> iterator_type;
    typedef MyIterator<const Tile<CC>*> const_iterator_type;

    iterator_type begin() { return iterator_type(*this); }

    const_iterator_type begin() const { return iterator_type(*this); }

    iterator_type end() { return iterator_type(*this,0,H); }

    const_iterator_type end() const { return const_iterator_type(*this, 0,H); }

    ~Grid()
    {
    }

    /**
     * Used to tell this Tile whether or not to actually execute any
     * events, or to just wait on any packet communication from other
     * Tiles instead.
     *
     * @param tileLoc The position of the Tile to set.
     *
     * @param value If value is true, this tells the Tile to begin
     *              executing its own events. Else, this Tile will
     *              only process Packets from other Tiles.
     */
    void SetTileToExecuteOnly(const SPoint& tileLoc, bool value);

    /**
     * Checks whether or not a specific Tile is currently executing
     * its own events or is simply processing Packets.
     *
     * @param tileLoc The location of the Tile in this Grid to check.
     */
    bool GetTileExecutionStatus(const SPoint& tileLoc);

    /**
     * Clears a Tile of all of its held atoms.
     *
     * @param tileLoc The location of the Tile in this Grid to clear.
     */
    void EmptyTile(const SPoint& tileLoc)
    {
      GetTile(tileLoc).ClearAtoms();
    }

    /**
     * Empties this Grid of all held Atoms.
     */
    void Clear();

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
     * caches) for the given siteInGrid.  Return false if there isn't
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
      return GetHeight() * Tile<CC>::OWNED_SIDE;
    }

    /**
     * Return the Grid width in (non-cache) sites
     */
    static u32 GetWidthSites()
    {
      return GetWidth() * Tile<CC>::OWNED_SIDE;
    }


    void Pause();

    void Unpause();

    void PlaceAtom(T& atom, const SPoint& location);

    const T* GetAtom(SPoint& loc)
    {
      SPoint tileInGrid, siteInTile;
      if (!MapGridToTile(loc, tileInGrid, siteInTile))
        FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
      return GetTile(tileInGrid).GetAtom(siteInTile);
    }

    void FillLastEventTile(SPoint& out);

    inline Tile<CC> & GetTile(const SPoint& pt)
    {return GetTile(pt.GetX(), pt.GetY());}

    inline const Tile<CC> & GetTile(const SPoint& pt) const
    {return GetTile(pt.GetX(), pt.GetY());}

    inline Tile<CC> & GetTile(u32 x, u32 y)
    { return m_tiles[x][y]; }

    inline const Tile<CC> & GetTile(u32 x, u32 y) const
    { return m_tiles[x][y]; }

    /* Don't count caches! */
    static inline const u32 GetTotalSites()
    { return GetWidthSites() * GetHeightSites(); }

    u64 GetTotalEventsExecuted() const;

    void WriteEPSImage(ByteSink & outstrm) const;

    void WriteEPSAverageImage(ByteSink & outstrm) const;

    void ResetEPSCounts();

    u32 GetAtomCount(ElementType atomType) const;

    void SurroundRectangleWithWall(s32 x, s32 y, s32 w, s32 h, s32 thickness);

    /**
     * Picks a random site in the grid, then sets all sites in a
     * randomly sized radius to Element_Empty .
     */
    void RandomNuke();

    /**
     * Randomly flips bits in randomly selected sites in this grid.
     */
    void XRay();

    u32 CountActiveSites();
  };
} /* namespace MFM */

#include "Grid.tcc"

#endif /*GRID_H*/
