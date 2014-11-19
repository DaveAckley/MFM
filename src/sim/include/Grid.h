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
#include "GridTransceiver.h"
#include "ElementRegistry.h"
#include "Logger.h"
#include <time.h>  /* For struct timespec, clock_gettime */

//#include "Element_Wall.h"

namespace MFM {

  /**
   * A two-dimensional grid of simulated Tiles.
   */
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

    void InitSeed();

    const u32 m_width, m_height;

    SPoint m_lastEventTile;

    Tile<CC> m_tiles[W][H];
    LonglivedLock m_intertileLocks[W][H][3]; // 3: E, SE, S == dir-Dirs::EAST

    /**
       Get the long-lived lock controlling cache activity going in
       direction dir from the Tile at (xtile,ytile) in the Grid.
     */
    LonglivedLock & GetIntertileLock(u32 xtile, u32 ytile, Dir dir) ;

    struct TileDriver {
      enum State { PAUSED, ADVANCING, EXIT_REQUEST };
      Mutex m_stateLock;
      State m_state;
      SPoint m_loc;
      Grid* m_gridPtr;
      pthread_t m_threadId;
      GridTransceiver m_channels[4]; // 4: NE, E, SE, S == dir-Dirs::NORTHEAST

      State GetState()
      {
        Mutex::ScopeLock lock(m_stateLock);
        return m_state;
      }

      void SetState(State newState)
      {
        Mutex::ScopeLock lock(m_stateLock);
        m_state = newState;
      }

      Tile<CC> & GetTile()
      {
        return m_gridPtr->GetTile(m_loc);
      }

    };
    TileDriver m_tileDrivers[W][H];
    bool m_threadsInitted;
    static void * TileDriverRunner(void *) ;

    bool m_backgroundRadiationEnabled;
    s32 m_warpFactor;

    ElementRegistry<CC> m_er;

    s32 m_xraySiteOdds;

    /**
     * A synchronized command sequence to the grid
     */
    struct TileDriverControl
    {
      virtual const char * GetName() = 0;

      virtual bool CheckPrecondition(TileDriver &) = 0;
      virtual void MakeRequest(TileDriver &) = 0;
      virtual bool CheckIfReady(TileDriver &) = 0;
      virtual void Execute(TileDriver &) = 0;

      /**
         Called once per op at the beginning
       */
      virtual void PreGridControl(Grid&) = 0;

      /**
         Called once per op at the end
       */
      virtual void PostGridControl(Grid&) = 0;
    };

    /**
     * Operations for synchronized grid pausing
     */
    struct PauseControl : public TileDriverControl
    {
      virtual const char * GetName()
      {
        return "Pause";
      }

      virtual bool CheckPrecondition(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        return !tile.IsOff();
      }

      virtual void MakeRequest(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        tile.RequestStatePassive();
      }
      virtual bool CheckIfReady(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        return tile.IsPassive();
      }
      virtual void Execute(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        tile.Pause();
      }

      virtual void PreGridControl(Grid& grid)
      {
      }

      virtual void PostGridControl(Grid& grid)
      {
        grid.SetGridRunning(false);
      }

    };

    /**
     * Operations for synchronized grid unpausing
     */
    struct RunControl : public TileDriverControl
    {

      virtual const char * GetName()
      {
        return "Unpause";
      }

      virtual bool CheckPrecondition(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        return !tile.IsActive();
      }

      virtual void MakeRequest(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        tile.NeedAtomRecount();
        tile.RequestStateActive();
      }
      virtual bool CheckIfReady(TileDriver & td)
      {
        Tile<CC> & tile = td.GetTile();
        return tile.IsActive();
      }
      virtual void Execute(TileDriver & td)
      {
        // We're already running; nothing to do
      }

      virtual void PreGridControl(Grid& grid)
      {
        grid.SetGridRunning(true);
      }

      virtual void PostGridControl(Grid& grid)
      {
      }

    };

    /**
     * Synchronized operations driver.
     */
    void DoTileDriverControl(TileDriverControl & tc);

  public:
    s32 GetWarpFactor() const
    {
      return m_warpFactor;
    }

    void SetWarpFactor(s32 wf)
    {
      if (wf < 0 || wf > 10)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      m_warpFactor = wf;
    }

    double GetAverageCacheRedundancy() const;
    void SetCacheRedundancy(u32 redundancyOddsType) ;

    void ReportGridStatus(Logger::Level level) ;

    Random& GetRandom() { return m_random; }

    bool* GetBackgroundRadiationEnabledPointer()
    {
      return &m_backgroundRadiationEnabled;
    }

    friend class GridRenderer;

    void SetSeed(u32 seed);

    Grid(ElementRegistry<CC>& elts)
      : m_seed(0)
      , m_width(W)
      , m_height(H)
      , m_threadsInitted(false)
      , m_backgroundRadiationEnabled(false)
      , m_er(elts)
      , m_xraySiteOdds(1000)
    {
      for (u32 y = 0; y < H; ++y)
      {
        for (u32 x = 0; x < W; ++x)
        {
          LOG.Debug("Tile[%d][%d] @ %p", x, y, &m_tiles[x][y]);
        }
      }
    }

    s32* GetXraySiteOddsPtr()
    {
      return &m_xraySiteOdds;
    }

    void Init();

    /**
       Init the 'TileDriver' threads.  This is done separately from
       (and should happen after) Init() so that we can use a Grid
       (e.g., for testing) without necessarily dealing with all the
       threading.
     */
    void InitThreads();

    /**
       Enable or disable the tiles and the transceivers.
     */
    void SetGridRunning(bool running) ;

    const Element<CC> * LookupElement(u32 elementType) const
    {
      return m_tiles[0][0].GetElementTable().Lookup(elementType);
    }

    ElementRegistry<CC>& GetElementRegistry()
    {
      return m_er;
    }

    void Needed(Element<CC> & anElement)
    {
      anElement.AllocateType();         // Force a type now
      m_er.RegisterElement(anElement);  // Make sure we're in here (How could we not?)

      for(u32 i = 0; i < W; i++)
      {
        for(u32 j = 0; j < H; j++)
        {
          m_tiles[i][j].RegisterElement(anElement);
        }
      }
      LOG.Message("Assigned type 0x%04x for %@",anElement.GetType(),&anElement.GetUUID());
    }

    /**
     * A minimal iterator over the Tiles of a grid.  Access via Grid::begin().
     */
    template <typename PointerType> class MyIterator
    {
      Grid & g;
      s32 i;
      s32 j;
    public:
      MyIterator(Grid<GC> & g, int i = 0, int j = 0) : g(g), i(i), j(j) { }

      bool operator!=(const MyIterator &m) const { return i != m.i || j != m.j; }
      void operator++()
      {
        if (j < g.H)
        {
          i++;
          if (i >= g.W)
          {
            i = 0;
            j++;
          }
        }
      }
      int operator-(const MyIterator &m) const
      {
        s32 rows = j-m.j;
        s32 cols = i-m.i;
        return rows*g.W + cols;
      }

      PointerType operator*() const
      {
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
    { }

    /**
     * Used to tell this Tile whether or not to actually execute any
     * events, or to just wait on any packet communication from other
     * Tiles instead.
     *
     * @param tileLoc The position of the Tile to set.
     *
     * @param value If \c value is \c true, this tells the Tile to begin
     *              executing its own events. Else, this Tile will
     *              only process Packets from other Tiles.
     */
    void SetTileEnabled(const SPoint& tileLoc, bool value);

    /**
     * Checks whether or not a specific Tile is currently executing
     * its own events or is simply processing Packets.
     *
     * @param tileLoc The location of the Tile in this Grid to check.
     */
    bool IsTileEnabled(const SPoint& tileLoc);

    /**
     * Clears a Tile of all of its held atoms.
     *
     * @param tileLoc The location of the Tile in this Grid to clear.
     */
    void EmptyTile(const SPoint& tileLoc)
    {
      Tile<CC> & tile = GetTile(tileLoc);
      tile.ClearAtoms();
    }

    /**
     * Empties this Grid of all held Atoms.
     */
    void Clear();

    /**
     * Based on the current connectivity pattern, check the visible
     * regions of each tile against the caches of its connected tiles.
     * Report debug messages for any discrepancies.
     */
    void CheckCaches();

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

    /**
     * Synchronize and pause the entire grid
     */
    void Pause()
    {
      PauseControl pc;
      DoTileDriverControl(pc);
    }

    /**
     * Synchronize and unpause the entire grid
     */
    void Unpause()
    {
      RunControl rc;
      DoTileDriverControl(rc);
    }

    /**
     * Resets all atom counts and refreshes the atoms counts in
     * every tile in the grid.
     */
    void RecountAtoms();

    void PlaceAtom(const T& atom, const SPoint& location);

    void XRayAtom(const SPoint& location);

    void MaybeXRayAtom(const SPoint& location);

    const T* GetAtom(SPoint& loc)
    {
      SPoint tileInGrid, siteInTile;
      if (!MapGridToTile(loc, tileInGrid, siteInTile))
      {
        LOG.Error("Can't get atom at site (%d,%d): Does not map to grid.",
                  loc.GetX(), loc.GetY());
        FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
      }
      return GetTile(tileInGrid).GetAtom(siteInTile);
    }

    T* GetWritableAtom(SPoint& loc)
    {
      SPoint tileInGrid, siteInTile;
      if (!MapGridToTile(loc, tileInGrid, siteInTile))
      {
        return NULL;
      }
      return GetTile(tileInGrid).GetWritableAtom(siteInTile);
    }

    void FillLastEventTile(SPoint& out);

    inline Tile<CC> & GetTile(const SPoint& pt)
    {
      if (!IsLegalTileIndex(pt))
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      return GetTile(pt.GetX(), pt.GetY());
    }

    inline const Tile<CC> & GetTile(const SPoint& pt) const
    {
      if (!IsLegalTileIndex(pt))
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      return GetTile(pt.GetX(), pt.GetY());
    }

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

    /**
     * Counts the number of sites which are occupied in this Grid and
     * gets a percentage, in the range [0.0 , 1.0] , describing the
     * sites which are occupied.
     *
     * @returns a percentage of the occupied sites in this Grid .
     */
    double GetEmptySitePercentage() const
    {
      return 1.0 - ((double)GetAtomCount(Element_Empty<CC>::THE_INSTANCE.GetType()) /
                    (double)(GetHeightSites() * GetWidthSites()));
    }

    //    void SurroundRectangleWithWall(s32 x, s32 y, s32 w, s32 h, s32 thickness);

    /**
     * Picks a random site in the grid, then sets all sites in a
     * randomly sized radius to Element_Empty .
     */
    void RandomNuke();

    /**
     * Sets whether or not background radiation will begin mutating
     * the Atoms of this Grid upon writing.
     */
    void SetBackgroundRadiation(bool value);

    /**
     * Toggles the background radiation in this Grid .
     */
    void ToggleBackgroundRadiation()
    {
      SetBackgroundRadiation(!m_backgroundRadiationEnabled);
    }

    /**
     * Checks to see if this Grid is currently administering
     * background radiataion.
     */
    bool IsBackgroundRadiataionEnabled()
    {
      return m_backgroundRadiationEnabled;
    }

    /**
     * Randomly flips bits in randomly selected sites in this grid.
     */
    void XRay();

    u32 CountActiveSites() const;
  };
} /* namespace MFM */

#include "Grid.tcc"

#endif /*GRID_H*/
