/* -*- mode:C++ -*- */
/**
  Tile.h An independent hardware unit capable of tiling space
  Copyright (C) 2014-2016 The Regents of the University of New Mexico.  All rights reserved.

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
  \file Tile.h An independent hardware unit capable of tiling space
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \lgpl
 */
#ifndef TILE_H
#define TILE_H

#include "Dirs.h"
#include "Util.h"    /* for COMPILATION_REQUIREMENT */
#include "Random.h"  /* for Random */
#include "Packet.h"
#include "Point.h"
#include "Element.h"
#include "Site.h"
#include "EventWindow.h"
#include "EventHistoryItem.h"
#include "ElementTable.h"
#include "CacheProcessor.h"
#include "UlamClassRegistry.h"
#include "LonglivedLock.h"
#include "OverflowableCharBufferByteSink.h"  /* for OString16 */
#include "LineCountingByteSource.h"

namespace MFM
{
#define IS_OWNED_CONNECTION(X) ((X) - Dirs::EAST >= 0 && (X) - Dirs::EAST < 4)

  template <class EC> class EventHistoryBuffer; // FORWARD

  /**
     The representation of a single indefinitely scalable hardware
     tile (currently simulated; hopefully soon also running natively).

     The Tile ctor is given the size of the Tile and the actual
     storage for the sites that hold Atoms.  The array size,
     therefore, isn't compiled-in, which does slow down some pretty
     inner-loopish functions, but also means that classes depending on
     Tile services do not have to be recompiled for each different
     Tile size.  In particular, Elements -- relying on Tile services
     such as EventWindow and Random -- can be separately compiled and
     dynamically loaded without knowing the intended Tile extent.

     A Tile is a state machine driven by the method Advance(), which
     may delegate to AdvanceComputation() to initiate an event (if the
     Tile is in State::ACTIVE), and to AdvanceCommunication() to
     perform packet processing (for its own events and/or events
     initiated by others).

   */
  template <class EC>  // An EventConfig
  class Tile
  {
  public:
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    typedef typename EC::SITE S;

    // Promote some parameter names
    enum { EVENT_WINDOW_RADIUS = EC::EVENT_WINDOW_RADIUS };

    /**
     * ELEMENT_TABLE_SIZE is the number of possible element types for
     * this EventConfig.
     */
    enum { ELEMENT_TABLE_SIZE = 1u<<AC::ATOM_TYPE_BITS };

    /**
     * The length of a side of this (necessarily square) Tile in sites.
     */
    const u32 TILE_SIDE;

    /**
     * The edge length of the portion of a Tile that is 'owned' by the
     * Tile itself -- i.e., excluding the cache boundary.
     */
    const u32 OWNED_SIDE;

    /**
     * An enumeration of the kinds of memory regions which may exist in
     * a Tile.
     */
    enum Region
    {
      REGION_CACHE   = 0,
      REGION_SHARED  = 1,
      REGION_VISIBLE = 2,
      REGION_HIDDEN  = 3,
      REGION_COUNT
    };

    Tile(const u32 tileSide, S * sites, const u32 eventbuffersize, EventHistoryItem * items) ;

    void SaveTile(ByteSink & to) const ;

    bool LoadTile(LineCountingByteSource & from) ;

    bool IsHistoryActive() const { return GetEventHistoryBuffer().IsHistoryActive(); }

    void SetHistoryActive(bool active) { return GetEventHistoryBuffer().SetHistoryActive(active); }

    const EventHistoryBuffer<EC> & GetEventHistoryBuffer() const { return m_eventHistoryBuffer; }

    EventHistoryBuffer<EC> & GetEventHistoryBuffer() { return m_eventHistoryBuffer; }

    /**
       Get the site-in-tile number of a given position \c index of the
       tile, \e including the caches, so index ranges from
       0..TILE_SIDE-1 in both x and y.  Return value is in
       0..(TILE_SIDE * TILE_SIDE - 1).  Fails if index x or y is
       greater than or equal to the tile side, or negative.
     */
    u32 GetSiteInTileNumber(const SPoint index) const
    {
      UPoint uidx = MakeUnsigned(index);
      MFM_API_ASSERT_ARG(uidx.GetX() < TILE_SIDE && uidx.GetY() < TILE_SIDE);
      return uidx.GetY()*TILE_SIDE + uidx.GetX();
    }

    /**
       Get the coordinate of a given \c siteInTileNumber in the tile,
       \e including the caches, so return value ranges from
       0..TILE_SIDE-1 in both x and y.  Return value is in 0..  1).
       Fails if siteInTileNumber is greater than or equal to TILE_SIDE
       * TILE_SIDE.  Returns SPoint (even though both x and y will be
       non-negative) to invert GetSiteInTileNumber()
     */
    SPoint GetCoordOfSiteInTileNumber(u32 siteInTileNumber) const
    {
      MFM_API_ASSERT_ARG(siteInTileNumber < TILE_SIDE*TILE_SIDE);
      return SPoint(siteInTileNumber % TILE_SIDE, siteInTileNumber / TILE_SIDE);
    }

    /**
       Get a const reference to the Site at position \c index of the
       tile, \e including the caches, so index ranges from
       0..TILE_SIDE-1 in both x and y.
     */
    const S & GetSite(const SPoint index) const
    {
      return m_sites[GetSiteInTileNumber(index)];
    }

    /**
       Get a mutable reference to the Site at position \c index of the
       tile, \e including the caches, so index ranges from
       0..TILE_SIDE-1 in both x and y.
     */
    S & GetSite(const SPoint index)
    {
      return const_cast<S &>(static_cast<const Tile<EC>*>(this)->GetSite(index));
    }

    /**
       Get a const reference to the Site at position \c index of the
       tile, \e excluding the caches, so index ranges from
       0..OWNED_SIDE-1 in both x and y.
     */
    const S & GetUncachedSite(const SPoint index) const
    {
      return GetSite(index + SPoint(EVENT_WINDOW_RADIUS,EVENT_WINDOW_RADIUS));
    }

    /**
       Get a mutable reference to the Site at position \c index of the
       tile, \e excluding the caches, so index ranges from
       0..OWNED_SIDE-1 in both x and y.
     */
    S & GetUncachedSite(const SPoint index)
    {
      return GetSite(index + SPoint(EVENT_WINDOW_RADIUS,EVENT_WINDOW_RADIUS));
    }

    /**
       Get the coordinate of a randomly selected 'owned' site in this
       tile.  An owned site is one that can be at the center of an
       event window that executes on this tile.
     */
    SPoint GetRandomOwnedCoord()
    {
      return OwnedCoordToTile(SPoint(GetRandom(), OWNED_SIDE, OWNED_SIDE));
    }

    u32 GetAtomCount(ElementType atomType) const
    {
      return m_cdata.GetAtomCount(atomType);
    }

    /**
     * The maximum number of tile parameters
     */
    enum { MAX_TILE_PARAMETERS = 16 };

    enum State
    {
      OFF,            // Not initiating events or processing packets
      PASSIVE,        // Processing packets but not initiating events
      ACTIVE          // Processing packets and initiating events
    };

    static const char * GetStateName(State state)
    {
      switch (state)
      {
      case OFF: return "OFF";
      case PASSIVE: return "PASSIVE";
      case ACTIVE: return "ACTIVE";
      default: return "illegal state";
      }
    }

    u32 GetWarpFactor() const
    {
      return m_warpFactor;
    }

    void SetWarpFactor(u32 warp)
    {
      m_warpFactor = MIN(10u, warp);
    }

  private:

    S * const m_sites;

    /**
     * A brief name or label for this Tile, for reporting and debugging
     */
    OString16 m_label;

    /** The top of the error stack for thread-local, per-Tile, FAIL
        processing. */
    MFMErrorEnvironmentPointer_t m_errorEnvironmentStackTop;

    /** The ElementTable instance which holds all atom behavior for this
        Tile. */
    ElementTable<EC> m_elementTable;

    /** The PRNG used for generating all random numbers in this Tile. */
    Random m_random;

    UlamClassRegistry<EC> m_ucr;

    s32 m_keyValues[MAX_TILE_PARAMETERS];

    void ClearTileParameters()
    {
      for (u32 i = 0; i < MAX_TILE_PARAMETERS; ++i)
      {
        m_keyValues[i] = S32_MIN;
      }
    }

    void CopyTileParameters(const Tile & hero)
    {
      for (u32 i = 0; i < MAX_TILE_PARAMETERS; ++i)
      {
        m_keyValues[i] = hero.m_keyValues[i];
      }
    }

    friend class EventWindow<EC>;
    friend class CacheProcessor<EC>;


  public:

    static SPoint TileCoordToOwned(const SPoint & tileCoord)
    {
      return tileCoord - SPoint(EVENT_WINDOW_RADIUS,EVENT_WINDOW_RADIUS);
    }

    static SPoint OwnedCoordToTile(const SPoint & ownedCoord)
    {
      return ownedCoord + SPoint(EVENT_WINDOW_RADIUS,EVENT_WINDOW_RADIUS);
    }

    /**
     * Gets the Region that a specified point is inside of. This
     * method fails upon an argument which does not point at a memory
     * region.
     *
     * @param pt The SPoint, which must be pointing at a memory region
     *           within this Tile, of which to find the region it is
     *           pointing at.
     *
     * @returns The Region that pt is pointing at.
     */
    Region RegionIn(const SPoint& pt);

    UlamClassRegistry<EC> & GetUlamClassRegistry() { return m_ucr; }

    const UlamClassRegistry<EC> & GetUlamClassRegistry() const { return m_ucr; }

    /**
     * A minimal iterator over the Sites of a tile.  Access via Tile::begin().
     */
    template <class SITETYPE, class TILETYPE> class TileIterator
    {
      TILETYPE & t;
      const u32 INDENT;
      s32 i;
      s32 j;
    public:
      TileIterator(TILETYPE & tile, u32 indent, int i, int j)
        : t(tile)
        , INDENT(indent)
        , i(i)
        , j(j)
      { }

      bool operator!=(const TileIterator &m) const
      {
        return &t != &m.t || i != m.i || j != m.j;
      }

      void operator++()
      {
        if (j < (s32) (t.TILE_SIDE - INDENT))
        {
          i++;
          if (i >= (s32) (t.TILE_SIDE - INDENT))
          {
            i = INDENT;
            j++;
          }
        }
      }

      /* NEED THIS?
      int operator-(const TileIterator &m) const
      {
        s32 rows = j-m.j;
        s32 cols = i-m.i;
        return rows*(t.TILE_SIDE - 2*INDENT) + cols;
      }
      */

      SITETYPE & operator*() const
      {
        return t.GetSite(AtSite());
      }

      SITETYPE * operator->() const
      {
        return &t.GetSite(AtSite());
      }

      /* AtSite() etc methods are always absolute full Tile coords */
      SPoint AtSite() const { return SPoint(GetXSite(),GetYSite()); }
      u32 GetXSite() const { return (u32) i; }
      u32 GetYSite() const { return (u32) j; }

      /* At() etc methods are relative to iteration origin (maybe all, maybe owned) */
      SPoint At() const { return SPoint(GetX(),GetY()); }
      u32 GetX() const { return (u32) (i - INDENT); }
      u32 GetY() const { return (u32) (j - INDENT); }

    };

    typedef TileIterator< S, Tile<EC> > iterator_type;
    typedef TileIterator< const S, const Tile<EC> > const_iterator_type;

    iterator_type beginAll() {
      return iterator_type(*this, 0, 0, 0);
    }
    const_iterator_type beginAll() const {
      return const_iterator_type(*this, 0, 0, 0);
    }

    iterator_type endAll() {
      return iterator_type(*this, 0, 0, TILE_SIDE);
    }
    const_iterator_type endAll() const {
      return const_iterator_type(*this, 0, 0, TILE_SIDE);
    }

    iterator_type beginOwned() {
      return iterator_type(*this, EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);
    }
    const_iterator_type beginOwned() const {
      return const_iterator_type(*this, EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);
    }

    iterator_type endOwned() {
      return iterator_type(*this, EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS, TILE_SIDE - EVENT_WINDOW_RADIUS);
    }
    const_iterator_type endOwned() const {
      return const_iterator_type(*this, EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS, TILE_SIDE - EVENT_WINDOW_RADIUS);
    }

    iterator_type begin(bool all) {
      if (all)
          return beginAll();
      return beginOwned();
    }
    const_iterator_type begin(bool all) const {
      if (all)
          return beginAll();
      return beginOwned();
    }

    iterator_type end(bool all) {
      if (all)
          return endAll();
      return endOwned();
    }
    const_iterator_type end(bool all) const {
      if (all)
          return endAll();
      return endOwned();
    }

  private:

    struct CountData {
      CountData(const Tile& t)
        : m_tile(t)
      { }

      const Tile & m_tile;

      /** A cache of the number of each type of Atom currently held
          within this Tile.  */
      u32 m_atomCount[ELEMENT_TABLE_SIZE];

      u32 m_illegalAtomCount;

      /** false when the m_atomCount counts are known valid.*/
      bool m_needRecount;

      void RecountAtoms() ;

      void RecountIfNeeded()
      {
        if (m_needRecount)
        {
          RecountAtoms();
        }
        m_needRecount = false;
      }

      u32 GetIllegalAtomCount()
      {
        RecountIfNeeded();
        return m_illegalAtomCount;
      }

      s32 GetAtomCount(u32 type) ;

      void NeedAtomRecount()
      {
        m_needRecount = true;
      }
    };

    /**
       CountData mutable so we can recompute it on the fly as needed.
     */
    mutable CountData m_cdata;

    /** Total times we tried to acquire a lock in this Tile */
    u64 m_lockAttempts;

    /** Total times we successfully acquired a lock in this Tile */
    u64 m_lockAttemptsSucceeded;

    /**
     * The coord of the last event (the one that caused
     * m_lastEventEventNumber to change most recently).
     */
    SPoint m_lastEventCenterOwned;

    EventWindow<EC> m_window;

    CacheProcessor<EC> m_cacheProcessors[Dirs::DIR_COUNT];
    RandomDirIterator m_dirIterator;

    bool AllCacheProcessorsIdle();

    /**
       Current state of the Tile state machine
     */
    State m_state;

    /**
       If tile is not enabled, it processes packets like passive but
       that's it.  In particular, a tile must be enabled before its
       state can be changed to ACTIVE.
     */
    bool m_enabled;

    /**
       true if atomic memory may be corrupted (on write) by background
       radiation.
     */
    bool m_backgroundRadiationEnabled;

    /**
       true if atomic memory may be corrupted (on read) by background
       radiation.
     */
    bool m_foregroundRadiationEnabled;

    enum BackgroundRadiationParameters
    {
      BACKGROUND_RADIATION_SITE_ODDS = 100,
      BACKGROUND_RADIATION_BIT_ODDS = 100
    };

    /**
       Mutex controlling access to m_state and m_requestedState
     */
    mutable Mutex m_stateAccess;

    /**
       Next state of the Tile state machine
     */
    State m_requestedState;

    /**
       How much event window selection should maximize the AER vs the
       flatness of space, in the range of 0..10.  Value 0 adds
       substantial pressure to distribute events evenly over sites;
       value 10 adds none.  Default value: 3.
     */
    u32 m_warpFactor;

    /**
       Record of recent past events for debugging and such
     */
    EventHistoryBuffer<EC> m_eventHistoryBuffer;

    /**
     * Compute the coordinates of \c atomLoc in a neighboring tile.
     * (There may or may not actually be a Tile in the given \c
     * neighbor direction; this method doesn't check or care.)
     *
     * @param neighbor The Dir code of the direction of the
     *                 neighboring Tile.
     *
     * @param atomLoc The point to map into the neighboring Tile's
     *                coordinates.
     * @return the mapped coordinate
     */
    SPoint GetNeighborLoc(Dir neighbor, const SPoint& atomLoc) const;

    /**
     * Gets the Region that a specified index, from the center of
     * the edge of a Tile, reaches to.
     *
     * @param index The index to check region membership of.
     *
     * @returns The Region which index will reach.
     */
    Region RegionFromIndex(const u32 index);

    /**
     * Performs a single Event on the generated EventWindow .
     */
    void DoEvent(bool locked, Dir lockRegion);

    /**
       Check if we are requested to change state, and if so, if we are
       able to do so.  Returns true no state change was requested, or
       it was requested and acted on successfully.  Returns false if
       we must block because a state change was requested but we could
       not currently perform it.
     */
    bool ConsiderStateChange() ;

    bool AdvanceComputation() ;

    /**
       Advance the passive packet processing state machine in the
       Tile.  Return true if any possibly valuable work was done.
     */
    bool AdvanceCommunication() ;

   public:
    void SetBackgroundRadiationEnabled(bool value);

    void SetForegroundRadiationEnabled(bool value);

    void SingleXRay(const SPoint & at, u32 bitOdds) ;

    void XRay(u32 siteOdds, u32 bitOdds) ;

    void Thin(u32 siteOdds) ;

    void SetCacheRedundancy(u32 redundancyOddsType)
    {
      for (u32 d = 0; d < Dirs::DIR_COUNT; ++d)
      {
        CacheProcessor<EC> & cp = m_cacheProcessors[d];
        cp.SetCacheRedundancy(redundancyOddsType);
      }
    }

    double GetAverageCacheRedundancy() const
    {
      u32 count = 0;
      double sumPercent = 0.0;

      for (u32 d = 0; d < Dirs::DIR_COUNT; ++d)
      {
        const CacheProcessor<EC> & cp = m_cacheProcessors[d];
        if (cp.IsConnected())
        {
          ++count;
          sumPercent += 100.0 / cp.GetCurrentCacheRedundancy();
        }
      }
      if (count == 0)
      {
        return -1;
      }
      return sumPercent / count;
    }

    /**
     * Flag that the atom counts in this tile may have changed
     */
    void NeedAtomRecount() const
    {
      m_cdata.NeedAtomRecount();
    }

    CacheProcessor<EC> & GetCacheProcessor(Dir toCache) ;

    const CacheProcessor<EC> & GetCacheProcessor(Dir toCache) const ;

    void CheckCacheFromDir(Dir direction, const Tile & otherTile) ;

    MFMErrorEnvironmentPointer_t * GetErrorEnvironmentStackTop()
    {
      return &m_errorEnvironmentStackTop;
    }

    bool IsActive() const
    {
      return GetCurrentState() == ACTIVE;
    }

    bool IsEnabled() const
    {
      return m_enabled;
    }

    bool IsOff() const
    {
      return GetCurrentState() == OFF;
    }

    /**
     * Sees if this Tile is currently in the passive mode.
     *
     * @returns \c true if this Tile is passive else \c false .
     */
    bool IsPassive()
    {
      return GetCurrentState() == PASSIVE;
    }


    State GetCurrentState() const
    {
      Mutex::ScopeLock lock(m_stateAccess);
      return m_state;
    }

    void SetRequestedState(State state) ;

    /**
       Advance the Tile state machine.  Return true if anything of
       possible value was accomplished.
     */
    bool Advance() ;

    /**
     * Gets the number of events executed within this Tile since
     * initialization.  Note that due to varying event window
     * boundaries this raw number of events will be much higher when a
     * tile is empty vs when occupied with high-radius elements.  By
     * contrast, GetEffectiveEventsExecuted() normalizes for occupancy
     * and radius.
     *
     * @returns The number of events executed within this Tile since
     *          initialization.
     *
     * @sa GetEffectiveEventsExecuted
     */
    u64 GetEventsExecuted() const
    {
      return m_window.GetEventWindowsExecuted();
    }

    u64 GetSitesAccessed() const
    {
      return m_window.GetSitesAccessed();
    }

    EventWindow<EC> & GetEventWindow()
    {
      return m_window;
    }

    const EventWindow<EC> & GetEventWindow() const
    {
      return m_window;
    }

    /**
     * Returns this Tile's label, if any.  May return an empty string,
     * never returns null.
     */
    const char * GetLabel() const
    {
      return m_label.GetZString();
    }

    void SetLabel(const char * label)
    {
      m_label.Reset();
      m_label.Print(label);
    }

    /**
     * Initializes an Tile
     */
    void Init();

    /** Copy tile parameters and other modifiable behavior from
        heroTile
     */
    void CopyHero(const Tile & heroTile)
    {
      CopyTileParameters(heroTile);
      SetWarpFactor(heroTile.GetWarpFactor());
      m_ucr = heroTile.m_ucr;

      const UlamClass<EC> * uempty = m_ucr.GetUlamElementEmpty();
      if (uempty)
      {
        const UlamElement<EC> * uelt = uempty->AsUlamElement();
        if (uelt)
        {
          m_elementTable.ReplaceEmptyElement(*uelt);
        }
      }
    }

    /**
     * Resets all Atoms and their counts to the Empty atom.
     */
    void ClearAtoms();

    /**
     * Connects a side of this Tile to an AbstractChannel
     *
     * @param channel The AbstractChannel to connect to.
     *
     * @param onSideA True if this Tile is side A of the AbstractChannel
     *
     * @param toCache The cache to share with other.
     */
    void Connect(AbstractChannel& channel, LonglivedLock & lock, Dir toCache) ;

    /**
     * Gets the number of sites in this Tile in sites, excluding caches.
     */
    u32 GetSites() const
    {
      return OWNED_SIDE*OWNED_SIDE;
    }

    /**
     * Gets the width of this Tile in sites, including caches.
     */
    u32 GetTileSide()
    {
      return TILE_SIDE;
    }

    /**
     * Gets a reference to this Tile's PRNG.
     * @Returns a reference to this Tile's PRNG.
     */
    Random & GetRandom();

    /**
     * Gets a reference to this Tile's ElementTable.
     *
     * @returns a reference to this Tile's ElementTable.
     */
    ElementTable<EC> & GetElementTable()
    {
      return m_elementTable;
    }

    const Element<EC> * ReplaceEmptyElement(const Element<EC>& newEmptyElement) ;

    /**
     * Returns an empty atom instance according to this Tile's ElementTable.
     */
    T GetEmptyAtom() const ;

    /**
     * Gets a const reference to this Tile's ElementTable.
     *
     * @returns a reference to this Tile's ElementTable.
     */
    const ElementTable<EC> & GetElementTable() const
    {
      return m_elementTable;
    }

    /**
     * Looks up an element type in this Tile's ElementTable.
     *
     * @returns a pointer to the Element associated with \a
     *          elementType, or NULL if there is no such Element.
     */
    const Element<EC> * GetElement(const u32 elementType) const
    {
      return m_elementTable.Lookup(elementType);
    }

    /**
     * Checks to see if this Tile has another tile in a specified
     * direction.
     *
     * @param dir The direction from this Tile in which to check for a
     *            neighboring Tile.
     *
     * @returns true if this Tile has a neighbor in direction dir, else false.
     */
    inline bool IsConnected(Dir dir) const;

    bool HasAnyConnections(Dir regionDir) const;

    /**
     * Finds the maximum ('square') distance from the center of the
     * Tile to this point.  This computation assumes the TILE_SIDE is
     * even (see ctor), and returns 1 for all of the four 'centermost'
     * sites.
     *
     * @param point The Point whose distance should be computed
     *
     * @returns The square distance from the Tile center to the given
     * site, greater than or equal to 1
     */
    u32 GetSquareDistanceFromCenter(const SPoint& point) const
    {
      // Do everything at double scale to get the rounding right
      const SPoint doubleResCenter(TILE_SIDE - 1, TILE_SIDE - 1);
      const SPoint doubleResPoint = point * 2;
      u32 doubleResDistance = (doubleResPoint - doubleResCenter).GetMaximumLength();
      return (doubleResDistance + 1) / 2;
    }

    /**
     * Checks to see if a specified local point is contained within a
     * Tile's caches.
     *
     * @param pt The local location which is in question of being inside
     *        of a cache.
     *
     * @returns true if pt is in a cache, else false.
     */
    bool IsInCache(const SPoint& point) const
    {
      u32 sqdist = GetSquareDistanceFromCenter(point);
      return sqdist <= TILE_SIDE / 2 && sqdist > TILE_SIDE / 2 - EVENT_WINDOW_RADIUS;
    }

    /**
     * Checks to see if a specified local point is contained within a
     * Tile's caches or shared.  Such points are potential targets for
     * writes via the cache protocol.
     *
     * @param pt The local location which is in question of being inside
     *        of a cache or a shared region.
     *
     * @returns true if pt is in a cache or a shared cache, else false.
     */
    bool IsInShared(const SPoint& point) const
    {
      u32 sqdist = GetSquareDistanceFromCenter(point);
      return sqdist <= TILE_SIDE / 2 && sqdist > TILE_SIDE / 2 - 2 * EVENT_WINDOW_RADIUS;
    }

    /**
       Is this cache location currently a possible event center on an
       adjacent tile?
       \return true iff is it
       \fails ILLEGAL_ARGUMENT if location is not a cache location
     */
    bool IsCacheSitePossibleEventCenter(const SPoint & location) const ;

    /**
     * Checks to see if a specified point is in Hidden memory, and therefore
     * does not require a lock to execute.
     *
     * @param pt The SPoint to check for Hidden memory membership.
     *
     * @returns true if pt is in Hidden memory.
     */
    bool IsInHidden(const SPoint& point) const
    {
      return GetSquareDistanceFromCenter(point) <= TILE_SIDE / 2 - 3 * EVENT_WINDOW_RADIUS;
    }

    /**
     * Checks to see if a specified local point is contained within a
     * Tile's owned sites (hidden + visible + shared).
     *
     * @param pt The local location which is in question of being inside
     *        of the Tile's own sites.
     *
     * @returns true if pt is an owned site of this Tile, else false.
     */
    bool IsOwnedSite(const SPoint & point) const
    {
      return GetSquareDistanceFromCenter(point) <= TILE_SIDE / 2 - 1 * EVENT_WINDOW_RADIUS;
    }

    /**
     * Checks to see if a specified local point is contained within a
     * Tile's sites, including caches.  If this method returns true,
     * then the GetAtom methods, applied to this location, will not
     * FAIL.
     *
     * @param pt The local location which is in question of being
     *           inside the bounds of this Tile
     *
     * @returns true if pt is a legal location in a Tile, else false.
     *
     * @sa GetAtom(const SPoint& pt)
     */
    inline bool IsInTile(const SPoint& pt) const
    {
      // Unsigned so possible negative coords wrap around to big positives
      return ((u32) pt.GetX()) < TILE_SIDE && ((u32) pt.GetY() < TILE_SIDE);
    }

    /**
     * Checks to see if a specified coordinate is contained within a
     * Tile's sites excluding the caches.  Indexing ignores the cache
     * boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).  If this method returns true, then
     * the 'Uncached' methods, applied to this location, will not FAIL.
     *
     * @param pt The coordinate which is in question of being a legal
     *           uncached site index in this Tile
     *
     * @returns true if pt is a legal uncached location in a Tile,
     * else false.
     *
     * @sa GetUncachedAtom(const SPoint& pt)
     * @sa GetUncachedAtom(s32 x, s32 y)
     * @sa GetUncachedSiteEvents(const SPoint site)
     */
    inline bool IsInUncachedTile(const SPoint& pt) const;

    /**
     * Checks to see if a specified local point is a site that
     * currently might receive events in this particular Tile.  Note
     * that liveness depends on the connectivity of this Tile.
     * Therefore, even given the same location as argument, the return
     * value from this method could change over time, if the
     * connectivity of this Tile changes.
     *
     * @param pt The local location which is in question of being
     *           a live site that receives events in this Tile
     *
     * @returns true if pt is a live site in this Tile, else false.
     */
    bool IsLiveSite(const SPoint & location) const;

    /**
     * Checks to see if a specified local point is a site that
     * currently might receive cache protocol updates in this
     * particular Tile.  The truth of this predicate depends in part
     * on the connectivity of this Tile.  Therefore, even given the
     * same location as argument, the return value from this method
     * could change over time, if the connectivity of this Tile
     * changes.
     *
     * @param pt The local location in question
     *
     * @returns true if pt currently might receive cache protocol
     * updates, given current connectivity.
     */
    bool IsReachableViaCacheProtocol(const SPoint & location) const;

    /**
     * Checks to see if a specified SPoint is in a given region of this
     * Tile (i.e. cache, shared, visible, or hidden).
     *
     * @param pt The SPoint to check region membership for.
     *
     * @param REACH The size, in sites from the edge of the tile, of
     *              this region. For example, a reach of R would
     *              specify searching for Atoms in this Tile's cache.
     *              2*R includes the shared region, and 3*R includes
     *              the visible region.
     *
     * @returns The direction of the cache pointed at by pt, or
     *          (Dir)-1 if pt is not pointing at a cache.
     */
    template <u32 REACH>
    Dir RegionAt(const SPoint& pt) const;

    Dir RegionAtReach(const SPoint& sp, const u32 reach) const ;

    /**
      Return ((Dir) -1) if no locks are needed to perform an event at
      pt, given an event window radius bounded by bound.  Otherwise
      return the 'center direction' in which locks are needed, and
      return true.  When GetLockDirection returns other than -1, if
      dir is an edge, only that lock is needed, and if dir is a
      corner, it and the two adjacent edges are all needed.
     */
    Dir GetLockDirection(const SPoint& pt, const u32 boundary) const
    {
      return RegionAtReach(pt,EVENT_WINDOW_RADIUS * 2 + boundary - 1);
    }

    /**
     * Finds the cache in this Tile which contains a specified SPoint.
     *
     * @param pt The Point which should specify the location of a cache.
     *
     * @returns The direction of the cache specified by pt, or
     *          (Dir)-1 if there is no such cache.
     */
    Dir CacheAt(const SPoint& pt) const;

    /**
     * Finds the region of cache or shared memory in this Tile which
     * contains a specified SPoint.
     *
     * @param pt The SPoint which should specify a location within
     *           cache or shared memory.
     *
     * @returns The direction of the cache or shared memory specified
     *          by pt, or (Dir)-1 if pt is not in cache or shared
     *          memory.
     */
    Dir SharedAt(const SPoint& pt) const;

    /**
     * Finds the region of cache, shared, or visible memory in this
     * Tile which contains a specified SPoint.
     *
     * @param pt The SPoint which should specify a location within
     *           cache, shared, or visible memory.
     *
     * @returns The direction of the cache, shared, or visible memory
     *          specified by pt, or (Dir)-1 if pt is not in cache,
     *          shared, or visible memory.
     */
    Dir VisibleAt(const SPoint& pt) const;

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     */
    const T* GetAtom(const SPoint & pt) const
    {
      return GetAtomInSite(false, pt);
    }

    const T* GetAtomInSite(bool getFromBase, const SPoint & pt) const
    {
      const S & site = GetSite(pt);
      if (getFromBase)
        return &site.GetBase().GetBaseAtom();
      else
        return &site.GetAtom();
    }

    /**
     * Gets an Atom from a specified point in this Tile, for the
     * purpose of filling an event window with it.  The distinction
     * between this and other 'GetAtom' methods is that if foreground
     * radiation is enabled, the result of GetAtomForEventWindow may
     * have been corrupted.
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A (possibly corrupted) copy of the Atom at location pt.
     */
    const T GetAtomForEventWindow(const SPoint & pt) const
    {
      const S & site = GetSite(pt);
      T atom = site.GetAtom();
      if (m_foregroundRadiationEnabled)
      {
        FAIL(INCOMPLETE_CODE);
      }
      return atom;
    }

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at the specified location.
     *
     * @remarks Because we need to be able to load an Atom's body
     *          through a configuration file, and this is the only
     *          place where Atoms are unique, we need to be able to
     *          access them in a writable way. Therefore, we have this
     *          non-const accessor. Use GetAtom if not writing to this
     *          Atom.
     */
    T* GetWritableAtom(const SPoint & pt)
    {
      S & site = GetSite(pt);
      return &site.GetAtom();
    }

    /**
     * Gets an Atom from a specified point in this Tile.  Indexing
     * ignores the cache boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     */
    const T* GetUncachedAtom(const SPoint& pt) const
    {
      return GetUncachedAtom(pt.GetX(), pt.GetY());
    }

    /**
     * Gets an Atom from a specified point in this Tile.  Indexing
     * ignores the cache boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param x The x coordinate of the location of the Atom to
     *          retrieve.

     * @param y The y coordinate of the location of the Atom to
     *          retrieve.
     *
     * @returns A pointer to the Atom at the specified location.
     */
    const T* GetUncachedAtom(s32 x, s32 y) const
    {
      return GetAtom(x + EVENT_WINDOW_RADIUS, y + EVENT_WINDOW_RADIUS);
    }

    void SaveSite(const SPoint &siteInTile, ByteSink& bs, AtomTypeFormatter<AC> & atf) const
    {
      GetSite(siteInTile).SaveConfig(bs,atf);
    }

    bool LoadSite(const SPoint &siteInTile, LineCountingByteSource& bs, AtomTypeFormatter<AC> & atf)
    {
      return GetSite(siteInTile).LoadConfig(bs,atf);
    }


    /**
     * Gets the site event count for a specified point in this Tile.
     * Indexing ignores the cache boundary, so possible range is (0,0)
     * to (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param site The coordinates of the location of the site whose
     *          event count should be retrieved.
     *
     * @param y The y coordinate of the location of the site whose
     *          event count should be retrieved.
     *
     * @returns The total events that have occured at that site
     */
    u64 GetUncachedSiteEvents(const SPoint site) const ;

    /**
     * Gets the 'write age' of a specified point in this Tile, up to
     * one billion.  Indexing ignores the cache boundary, so possible
     * range is (0,0) to (OWNED_SIDE-1,OWNED_SIDE-1).  The write age
     * is the number of events on this tile since the contents of the
     * specified point changed.  To obtain AEPS, divide this value by
     * Tile::GetSites().
     *
     * @param site The coordinates of the location of the site whose
     *          write age should be retrieved.
     *
     * @returns The events since that site's content changed, if less
     * than one billion, else one billion.
     *
     * @sa GetUncachedWriteAge
     *
     */
    u32 GetUncachedWriteAge32(const SPoint site) const ;

    /**
     * Gets the 'write age' of a specified point in this Tile.
     * Indexing ignores the cache boundary, so possible range is (0,0)
     * to (OWNED_SIDE-1,OWNED_SIDE-1).  The write age is the number of
     * events on this tile since the contents of the specified point
     * changed.  To obtain AEPS, divide this value by
     * Tile::GetSites().
     *
     * @param site The coordinates of the location of the site whose
     *          write age should be retrieved.
     *
     * @returns The events since that site's content changed
     *
     * @sa GetUncachedWriteAge32
     *
     */
    u64 GetUncachedWriteAge(const SPoint site) const ;

    /**
     * Gets the 'event age' of a specified point in this Tile, up to
     * one billion.  Indexing ignores the cache boundary, so possible
     * range is (0,0) to (OWNED_SIDE-1,OWNED_SIDE-1).  The event age
     * is the number of events on this tile since the last event at
     * the specified point changed.  To obtain AEPS, divide this value
     * by Tile::GetSites().
     *
     * @param site The coordinates of the location of the site whose
     *          event age should be retrieved.
     *
     * @returns The events since that site's content changed, if less
     * than one billion, else one billion
     *
     * @sa GetUncachedEventAge
     */
    u32 GetUncachedEventAge32(const SPoint site) const ;

    /**
     * Gets the 'event age' of a specified point in this Tile.
     * Indexing ignores the cache boundary, so possible range is (0,0)
     * to (OWNED_SIDE-1,OWNED_SIDE-1).  The event age is the number of
     * events on this tile since the last event at the specified point
     * changed.  To obtain AEPS, divide this value by
     * Tile::GetSites().
     *
     * @param site The coordinates of the location of the site whose
     *          event age should be retrieved.
     *
     * @returns The events since that site's content changed
     *
     * @sa GetUncachedEventAge32
     */
    u64 GetUncachedEventAge(const SPoint site) const ;

    /**
     * Fills a specified Point with the coordinates of the last EventWindow center.
     *
     * @param out The Point to fill with the coordinates of the last
     *            EventWindow center. All previous data held within this
     *            Point will be erased.
     */
    void FillLastExecutedAtom(SPoint& out)
    {
      out = m_lastEventCenterOwned;
    }

    /**
     * Store an atom anywhere in the Tile, using the 'raw' coordinate
     * system in which (0,0) is in the Tile cache.
     *
     * @param atom The Atom to place within this Tile.
     * @param pt The local location in which to store atom.
     * @sa PlaceInternalAtom
     */
    void PlaceAtom(const T& atom, const SPoint& pt)
    {
      PlaceAtomInSite(false, atom, pt);
    }

    void PlaceAtomInSite(bool placeInBase, const T& atom, const SPoint& pt);

    /**
     * Store and/or consistency check an atom against the current
     * contents of site, which is in full-Tile coordinates and must
     * correspond to a cache location.  Returns true iff the request
     * was consistent, meaning isDifferent is true and the atom was
     * different, or isDifferent is false and the atom matched what
     * was already in the site.
     *
     * @param atom The Atom to place within this Tile.
     * @param pt The cache location in which to store or check atom.
     *
     * @returns true for a consistent update, false for inconsistent
     * update
     *
     * @fails ILLEGAL_ARGUMENT if site is not a cache location
     */
    bool ApplyCacheUpdate(bool isDifferent, const T& atom, const SPoint& site);


    /**
     * Store an atom in the 'locally-owned' portion of a Tile, using the
     * 'natural' coordinate system in which (0,0) is the upper left of
     * the sites that this Tile owns.
     *
     * @param atom The Atom to place within this Tile.
     *
     * @param pt The local location in which to store atom, beginning in
     *           this Tile's main memory.
     *
     * @sa PlaceAtom
     */
    void PlaceInternalAtom(const T& atom, const SPoint& pt)
    {
      PlaceAtom(atom,pt+SPoint(EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS));
    }

    /**
     * Scan this Tile's GetSites() sites and increment the count
     * associated with each atom found.
     */
    void IncrementAtomHistogram(s32 count[ELEMENT_TABLE_SIZE]);

    /**
     * Sees if this Tile is currently running.
     *
     * @returns \c true if this Tile is running else \c false .
     */
    bool IsRunning()
    {
      FAIL(INCOMPLETE_CODE);
    }

    /**
     * Sees if this Tile is ready to be paused. There is some
     * processing to be done between \c PauseRequested() and \c
     * Pause() , so this method tells whether or not it is time to
     * pause each Tile .
     *
     * @returns \c true if this Tile is ready to be paused, else \c
     *          false .
     */
    /*
    bool IsPauseReady()
    {
      return GetCurrentState() == ACTIVE;
    }
    */

    void SetDisabled()
    {
      m_enabled = false;
    }

    void SetEnabled()
    {
      m_enabled = true;
    }

    void Pause()
    {
      if (GetCurrentState() != PASSIVE)
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    void RequestStateActive()
    {
      SetRequestedState(ACTIVE);
    }

    void RequestStatePassive()
    {
      SetRequestedState(PASSIVE);
    }

    void RequestStateOff()
    {
      SetRequestedState(OFF);
    }

    void Run()
    {
      if (GetCurrentState() != ACTIVE)
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    void ReportTileStatus(Logger::Level level) ;

    /**
     * Registers an Element into this Tile's ElementTable.
     *
     * @param the Element to register into this Tile's ElementTable.
     */
    void RegisterElement(const Element<EC> & anElement)
    {
      m_elementTable.RegisterElement(anElement);
    }

  public:

    /**
     * Sets a parameter in this Tile
     *
     * @param key a small u32 specifying the parameter slot to set
     *
     * @param value the new value to place in the key slot
     */
    void SetTileParameter(const u32 key, const s32 value)
    {
      MFM_API_ASSERT_ARG(key < MAX_TILE_PARAMETERS);
      m_keyValues[key] = value;
    }

    /**
     * Gets a parameter in this Tile
     *
     * @param key a small u32 specifying the parameter slot to get
     *
     * @returns the current value in the key slot
     */
    s32 GetTileParameter(const u32 key)
    {
      MFM_API_ASSERT_ARG(key < MAX_TILE_PARAMETERS);
      return m_keyValues[key];
    }

  };
} /* namespace MFM */

#include "Tile.tcc"

#endif /*TILE_H*/
