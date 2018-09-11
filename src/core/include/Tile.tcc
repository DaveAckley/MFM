/* -*- C++ -*- */
#include "MDist.h"
#include "Element_Empty.h"
#include "Logger.h"
#include "AtomSerializer.h"
#include "EventHistoryBuffer.h"

#include "Util.h"

namespace MFM
{
  template <class EC>
  Tile<EC>::Tile(const u32 tileWidth, const u32 tileHeight, const GridLayoutPattern gridlayout, S * sites, const u32 eventbuffersize, EventHistoryItem * items)
    : TILE_WIDTH(tileWidth)
    , TILE_HEIGHT(tileHeight)
    , OWNED_WIDTH(TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS)  // This OWNED_SIDE computation is duplicated in Grid.h!
    , OWNED_HEIGHT(TILE_HEIGHT - 2 * EVENT_WINDOW_RADIUS)  // This OWNED_SIDE computation is duplicated in Grid.h!
    , GRID_LAYOUT(gridlayout)
    , DUMMY_TILE(false)
    , m_sites(sites)
    , m_cdata(*this)
    , m_lockAttempts(0)
    , m_lockAttemptsSucceeded(0)
    , m_window(*this)
    , m_dirIterator(Dirs::DIR_COUNT)
    , m_state(OFF)
    , m_enabled(true)
    , m_backgroundRadiationEnabled(false)
    , m_foregroundRadiationEnabled(false)
    , m_requestedState(OFF)
    , m_warpFactor(3)
    , m_eventHistoryBuffer(*this, eventbuffersize, items)
  {
    // TILE sides can't be too small, and we must apparently have sites, but not necessarily hidden ones.
    // Effort to avoid simultaneous locks in opposite directions (e.g. East and West);
    MFM_API_ASSERT_ARG(TILE_WIDTH >= 6*EVENT_WINDOW_RADIUS && TILE_HEIGHT >= 6*EVENT_WINDOW_RADIUS && m_sites != 0);

    // Require even TILE side dimensions.
    MFM_API_ASSERT_ARG(2 * TILE_WIDTH / 2 == TILE_WIDTH);
    MFM_API_ASSERT_ARG(2 * TILE_HEIGHT / 2 == TILE_HEIGHT);

    //staggered grid layout ignores NORTH & SOUTH directions
    if(IsTileGridLayoutStaggered())
      {
	static u32 staggeredindexes[Dirs::DIR_COUNT];
	u32 counter = 0;
	for(u32 d=0; d < Dirs::DIR_COUNT; d++)
	  {
	    if(Dirs::IsValidDir(d, true))
	      {
		staggeredindexes[counter] = d;
		counter++;
	      }
	    //else skip this direction.
	  }
	m_dirIterator.Reinit(counter, staggeredindexes);
	MFM_API_ASSERT_STATE(counter == m_dirIterator.GetLimit());
      }

    Init();
  }

  template <class EC>
  Tile<EC>::~Tile() {/* defined to avoid inline error */}

  template <class EC>
  void Tile<EC>::SaveTile(ByteSink & to) const
  {
    to.Printf(",");
    to.Print(m_lockAttempts,Format::LXX64);
    to.Print(m_lockAttemptsSucceeded,Format::LXX64);
    to.Print(m_window.GetEventWindowsExecuted(),Format::LXX64);
    to.Print(m_window.GetEventWindowsAttempted(),Format::LXX64);
    to.Printf(",%D%D%D",
              m_enabled,
              m_backgroundRadiationEnabled,
              m_warpFactor);
  }

  template <class EC>
  bool Tile<EC>::LoadTile(LineCountingByteSource & from)
  {
    if (1 != from.Scanf(",")) return false;

    u64 tmp_m_lockAttempts;
    u64 tmp_m_lockAttemptsSucceeded;
    u64 tmp_EventWindowsExecuted;
    u64 tmp_EventWindowsAttempted;
    u32 tmp_m_enabled;
    u32 tmp_m_backgroundRadiationEnabled;
    u32 tmp_m_warpFactor;
    if (!from.Scan(tmp_m_lockAttempts,Format::LXX64)) return false;
    if (!from.Scan(tmp_m_lockAttemptsSucceeded,Format::LXX64)) return false;
    if (!from.Scan(tmp_EventWindowsExecuted,Format::LXX64)) return false;
    if (!from.Scan(tmp_EventWindowsAttempted,Format::LXX64)) return false;
    if (4 != from.Scanf(",%D%D%D",
                        &tmp_m_enabled,
                        &tmp_m_backgroundRadiationEnabled,
                        &tmp_m_warpFactor))
      return false;

    m_lockAttempts = tmp_m_lockAttempts;
    m_lockAttemptsSucceeded = tmp_m_lockAttemptsSucceeded;
    m_window.SetEventWindowsExecuted(tmp_EventWindowsExecuted);
    m_window.SetEventWindowsAttempted(tmp_EventWindowsAttempted);
    m_enabled = tmp_m_enabled;
    m_backgroundRadiationEnabled = tmp_m_backgroundRadiationEnabled;
    m_warpFactor = tmp_m_warpFactor;

    return true;

  }


  template <class EC>
  void Tile<EC>::Init()
  {
    m_elementTable.Reinit();

    Element_Empty<EC>::THE_INSTANCE.AllocateEmptyType();
    RegisterElement(Element_Empty<EC>::THE_INSTANCE);

    ClearAtoms();
    ClearTileParameters();

    m_dirIterator.Shuffle(m_random);

  }

  template <class EC>
  const Element<EC> * Tile<EC>::ReplaceEmptyElement(const Element<EC>& newEmptyElement)
  {
    return m_elementTable.ReplaceEmptyElement(newEmptyElement);
  }

  template <class EC>
  typename EC::ATOM_CONFIG::ATOM_TYPE Tile<EC>::GetEmptyAtom() const
  {
    return Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom();
  }

  template <class EC>
  void Tile<EC>::SingleXRay(const SPoint & at, u32 bitOdds)
  {
    Random & random = GetRandom();
    GetWritableAtom(at)->XRay(random, bitOdds);
  }

  template <class EC>
  void Tile<EC>::XRay(u32 siteOdds, u32 bitOdds)
  {
    Random & random = GetRandom();
    for(iterator_type i = beginAll(); i != endAll(); ++i) { // hitting caches too
      if (random.OneIn(siteOdds))
        i->GetAtom().XRay(random, bitOdds);
    }
  }

  template <class EC>
  void Tile<EC>::Thin(u32 siteOdds)
  {
    Random & random = GetRandom();
    for(iterator_type i = beginOwned(); i != endOwned(); ++i) {
      if (random.OneIn(siteOdds))
        i->Clear();
    }
  }

  template <class EC>
  void Tile<EC>::ClearAtoms()
  {
    for(iterator_type i = beginAll(); i != endAll(); ++i) {
      i->Clear();
    }
    NeedAtomRecount();
  }

  template <class EC>
  void Tile<EC>::Connect(AbstractChannel& channel, LonglivedLock & lock, Dir toCache)
  {
    CacheProcessor<EC> & cxn = GetCacheProcessor(toCache);

    MFM_API_ASSERT_STATE(!cxn.IsConnected());

    cxn.ClaimCacheProcessor(*this, channel, lock, toCache);
  }

  template <class EC>
  CacheProcessor<EC> & Tile<EC>::GetCacheProcessor(Dir toCache)
  {
    return const_cast<CacheProcessor<EC>&>(static_cast<const Tile<EC>*>(this)->GetCacheProcessor(toCache));
  }

  template <class EC>
  const CacheProcessor<EC> & Tile<EC>::GetCacheProcessor(Dir toCache) const
  {
    MFM_API_ASSERT_ARG(Dirs::IsLegalDir(toCache));
    return m_cacheProcessors[toCache];
  }

  template <class EC>
  Random& Tile<EC>::GetRandom()
  {
    return m_random;
  }

  template <class EC>
  s32 Tile<EC>::CountData::GetAtomCount(u32 type)
  {
    s32 idx = m_tile.m_elementTable.GetIndex(type);
    if (idx < 0)
      return -1;

    RecountIfNeeded();

    return m_atomCount[idx];
  }

  template <class EC>
  void Tile<EC>::CountData::RecountAtoms()
  {
    for(u32 i = 0; i < ELEMENT_TABLE_SIZE; i++) m_atomCount[i] = 0;

    m_illegalAtomCount = 0;

    for(const_iterator_type i = m_tile.beginOwned(); i != m_tile.endOwned(); ++i) {

      u32 atype = i->GetAtom().GetType();
      s32 idx = m_tile.m_elementTable.GetIndex(atype);

      if (idx < 0) ++m_illegalAtomCount;
      else ++m_atomCount[idx];
    }
  }

  template <class EC>
  u32 Tile<EC>::GetUncachedWriteAge32(const SPoint site) const
  {
    const u32 A_BILLION = 1000*1000*1000;
    u64 age = GetUncachedWriteAge(site);
    if (age > A_BILLION) return A_BILLION;
    return (u32) age;
  }

  template <class EC>
  u64 Tile<EC>::GetUncachedWriteAge(const SPoint at) const
  {
    MFM_API_ASSERT_ARG(IsInUncachedTile(at));
    /**
    return
      GetEventsExecuted() -
      GetUncachedSite(at).GetLastChangedEventNumber();
    */
    return GetUncachedSite(at).GetWriteAge();
}

  template <class EC>
  u64 Tile<EC>::GetUncachedEventAge(const SPoint at) const
  {
    MFM_API_ASSERT_ARG(IsInUncachedTile(at));

    /* XXX
    return
      (u64) ((s64)  -
             GetUncachedSite(at).GetLastEventEventNumber());
    */
    return GetUncachedSite(at).GetEventAge(GetEventsExecuted());
  }


  template <class EC>
  u32 Tile<EC>::GetUncachedEventAge32(const SPoint site) const
  {
    const u32 A_BILLION = 1000*1000*1000;
    u64 age = GetUncachedEventAge(site);
    if (age > A_BILLION) return A_BILLION;
    return (u32) age;
  }

  template <class EC>
  u64 Tile<EC>::GetUncachedSiteEvents(const SPoint at) const
  {
    MFM_API_ASSERT_ARG(IsInUncachedTile(at));
    return GetUncachedSite(at).GetEventCount();
  }

#if 0
  template <class CC>
  void Tile<CC>::InternalPutAtom(const T & atom, s32 x, s32 y)
  {
    if (((u32) x) >= TILE_WIDTH || ((u32) y) >= TILE_WIDTH)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    }
    m_atoms[x][y] = atom;
  }
#endif

  template <class EC>
  template <u32 REACH>
  Dir Tile<EC>::RegionAt(const SPoint& sp) const
  {
    THREEDIR tmpdirs;
    u32 count = RegionAtReach(sp, REACH, tmpdirs);
    MFM_API_ASSERT_STATE(count <= MAX_LOCKS_NEEDED);
    return tmpdirs[0]; //return the first only (deprecated)
  }

  template <class EC>
  u32 Tile<EC>::RegionAtReach(const SPoint& sp, const u32 REACH, THREEDIR& rtndirs, bool onlyConnected) const
  {
    u32 rtncount = 0;
    bool isStaggered = IsTileGridLayoutStaggered();

    if(isStaggered && !CanMakeUnsigned(sp)) return 0;  //Tue Sep 11 09:05:54 2018 esa

    UPoint pt = MakeUnsigned(sp);

    if(pt.GetX() < REACH)
      {
	if(pt.GetY() < REACH)
	  {
	    //NW corner
	    TryToAddRegionAtReach(Dirs::NORTHWEST, rtncount, rtndirs, onlyConnected);

	    TryToAddRegionAtReach(Dirs::WEST, rtncount, rtndirs, onlyConnected);

	    if(!isStaggered)
	      TryToAddRegionAtReach(Dirs::NORTH, rtncount, rtndirs, onlyConnected);
	  }
	else if(pt.GetY() >= TILE_HEIGHT - REACH)
	  {
	    //SW corner
	    TryToAddRegionAtReach(Dirs::SOUTHWEST, rtncount, rtndirs, onlyConnected);

	    TryToAddRegionAtReach(Dirs::WEST, rtncount, rtndirs, onlyConnected);

	    if(!isStaggered)
	      TryToAddRegionAtReach(Dirs::SOUTH, rtncount, rtndirs, onlyConnected);
	  }
	else
	  {
	    TryToAddRegionAtReach(Dirs::WEST, rtncount, rtndirs, onlyConnected);
	  }
      }
    else if(pt.GetX() >= TILE_WIDTH - REACH)
      {
	if(pt.GetY() < REACH)
	  {
	    //NE corner
	    TryToAddRegionAtReach(Dirs::NORTHEAST, rtncount, rtndirs, onlyConnected);

	    TryToAddRegionAtReach(Dirs::EAST, rtncount, rtndirs, onlyConnected);

	    if(!isStaggered)
	      TryToAddRegionAtReach(Dirs::NORTH, rtncount, rtndirs, onlyConnected);
	  }
	else if(pt.GetY() >= TILE_HEIGHT - REACH)
	  {
	    //SE corner
	    TryToAddRegionAtReach(Dirs::SOUTHEAST, rtncount, rtndirs, onlyConnected);

	    TryToAddRegionAtReach(Dirs::EAST, rtncount, rtndirs, onlyConnected);

	    if(!isStaggered)
	      TryToAddRegionAtReach(Dirs::SOUTH, rtncount, rtndirs, onlyConnected);
	  }
	else
	  {
	    TryToAddRegionAtReach(Dirs::EAST, rtncount, rtndirs, onlyConnected);
	  }
      }
    else
      {
	if(isStaggered)
	  {
	    if((pt.GetX() >= TILE_WIDTH/2 - REACH) && (pt.GetX() < TILE_WIDTH/2 + REACH))
	      {
		//in the middle +/- REACH
		if(pt.GetY() < REACH)
		  {
		    TryToAddRegionAtReach(Dirs::NORTHWEST, rtncount, rtndirs, onlyConnected);
		    TryToAddRegionAtReach(Dirs::NORTHEAST, rtncount, rtndirs, onlyConnected);
		  }
		else if(pt.GetY() >= TILE_HEIGHT - REACH)
		  {
		    TryToAddRegionAtReach(Dirs::SOUTHWEST, rtncount, rtndirs, onlyConnected);
		    TryToAddRegionAtReach(Dirs::SOUTHEAST, rtncount, rtndirs, onlyConnected);
		  }
		//else
	      }
	    else if(pt.GetX() < TILE_WIDTH/2)
	      {
		if(pt.GetY() < REACH)
		  {
		    TryToAddRegionAtReach(Dirs::NORTHWEST, rtncount, rtndirs, onlyConnected);
		  }
		else if(pt.GetY() >= TILE_HEIGHT - REACH)
		  {
		    TryToAddRegionAtReach(Dirs::SOUTHWEST, rtncount, rtndirs, onlyConnected);
		  }
	      }
	    else
	      {
		MFM_API_ASSERT_STATE(pt.GetX() >=TILE_WIDTH/2); //sanity
		if(pt.GetY() < REACH)
		  {
		    TryToAddRegionAtReach(Dirs::NORTHEAST, rtncount, rtndirs, onlyConnected);
		  }
		else if(pt.GetY() >= TILE_HEIGHT - REACH)
		  {
		    TryToAddRegionAtReach(Dirs::SOUTHEAST, rtncount, rtndirs, onlyConnected);
		  }
	      }
	  }
	else
	  {
	    //checkerboard configuration
	    // X in neither east nor west reach
	    if(pt.GetY() < REACH)
	      {
		TryToAddRegionAtReach(Dirs::NORTH, rtncount, rtndirs, onlyConnected);
	      }
	    else if(pt.GetY() >= TILE_HEIGHT - REACH)
	      {
		TryToAddRegionAtReach(Dirs::SOUTH, rtncount, rtndirs, onlyConnected);
	      }
	  }
      }
    return rtncount;
  } //RegionAtReach

  template <class EC>
  void Tile<EC>::TryToAddRegionAtReach(Dir d, u32& rtncount, THREEDIR & rtndirs, bool onlyConnected) const
  {
    if(!onlyConnected || IsConnected(d))
      {
	rtndirs[rtncount] = d;
	rtncount++;
      }
  } //private helper

  template <class EC>
  u32 Tile<EC>::CacheAt(const SPoint& pt, THREEDIR & rtndirs, const bool onlyConnected) const
  {
    return RegionAtReach(pt, EVENT_WINDOW_RADIUS, rtndirs, onlyConnected);
  }

  template <class EC>
  u32 Tile<EC>::SharedAt(const SPoint& pt, THREEDIR & rtndirs, const bool onlyConnected) const
  {
    return RegionAtReach(pt, EVENT_WINDOW_RADIUS * 2, rtndirs, onlyConnected);
  }

  template <class EC>
  u32 Tile<EC>::VisibleAt(const SPoint& pt, THREEDIR & rtndirs, const bool onlyConnected) const
  {
    return RegionAtReach(pt, EVENT_WINDOW_RADIUS * 3, rtndirs, onlyConnected);
  }

  template <class EC>
  bool Tile<EC>::ApplyCacheUpdate(const bool isDifferent, const T& atom, const SPoint& site)
  {
    MFM_API_ASSERT_ARG(!IsInHidden(site));  // That would make no sense

    if(!IsInShared(site))
      FAIL(ILLEGAL_ARGUMENT); //per description in Tile.h

    bool consistent;
    const T& oldAtom = *GetAtom(site);

    if (atom != oldAtom)
    {
      PlaceAtom(atom, site);

      consistent = isDifferent;
    }
    else
    {
      consistent = !isDifferent;
    }

    return consistent;
  }

  template <class EC>
  void Tile<EC>::PlaceAtomInSite(bool placeInBase, const T& atom, const SPoint& pt, bool doIdenticalCheck)
  {
    if(!doIdenticalCheck)
      MFM_LOG_DBG6(("Tile %s: Place AtomInSite type %04x at (%2d,%2d)",
		    this->GetLabel(),
		    atom.GetType(),
		    pt.GetX(), pt.GetY()));

    if (!IsLiveSite(pt))
    {
      if (atom.GetType() != Element_Empty<EC>::THE_INSTANCE.GetType())
      {
	MFM_LOG_DBG4(("Tile %s: NOT placing AtomInSite type %04x at (%2d,%2d)",
		      this->GetLabel(),
		      atom.GetType(),
		      pt.GetX(), pt.GetY()));
      }
      return;
    }

    Site<AC> & site = GetSite(pt);
    T & oldAtom = placeInBase ? site.GetBase().GetBaseAtom() : site.GetAtom();
    T newAtom = atom;
    unwind_protect(
    {
      oldAtom.SetEmpty();
      LOG.Warning("Tile %s: failure during place AtomInSite type %04x at (%2d,%2d) erased",
		  this->GetLabel(),
		  atom.GetType(),
		  pt.GetX(), pt.GetY());
    },
    {
      if(m_backgroundRadiationEnabled &&
         m_random.OneIn(BACKGROUND_RADIATION_SITE_ODDS))
      {
        // Write fault!
        newAtom.XRay(m_random, BACKGROUND_RADIATION_BIT_ODDS);
      }

      bool owned = IsOwnedSite(pt);

      if (oldAtom != newAtom)
	{
	  if(doIdenticalCheck)
	    {
	      AtomSerializer<AC> oldas(oldAtom);
	      AtomSerializer<AC> as(newAtom);

	      LOG.Warning("Tile %s: doIdenticalCheck failure during place AtomInSite type [%04x/%@] was [%04x/%@] at (%2d,%2d)",
			  this->GetLabel(),
			  newAtom.GetType(), &as,
			  oldAtom.GetType(), &oldas,
			  pt.GetX(), pt.GetY());
	    }
	  else
	    {
	      NeedAtomRecount();
	      if (owned)
		site.MarkChanged();

	      oldAtom = newAtom;
	    }
	}
    });
  }

  template <class EC>
  bool Tile<EC>::IsConnected(Dir dir) const
  {
    const CacheProcessor<EC> & cxn = GetCacheProcessor(dir);
    //return cxn.IsConnected();
    return !cxn.IsUnclaimed() && cxn.IsConnected();
  }

  template <class EC>
  bool Tile<EC>::IsReachableViaCacheProtocol(const SPoint & location) const
  {
    if (!IsInShared(location))
    {
      return false;
    }

    THREEDIR shareddirs;
    u32 sharedcnt = SharedAt(location, shareddirs, NOCHKCONNECT);

    if(sharedcnt > 1)
      {
	//then must be a corner
	THREEDIR connecteddirs;
	u32 connectedcnt = SharedAt(location, connecteddirs, YESCHKCONNECT);
	return (sharedcnt == connectedcnt);
      }
    return false;
  }

  template <class EC>
  bool Tile<EC>::IsCacheSitePossibleEventCenter(const SPoint & location) const
  {
    MFM_API_ASSERT_ARG(IsInCache(location));
    THREEDIR cnCacheDirs;
    u32 count = CacheAt(location, cnCacheDirs, YESCHKCONNECT);
    bool isInANeighborsShared = false;
    for(u32 i = 0; i < count; i++)
      {
	Dir dir = cnCacheDirs[i];
	const CacheProcessor<EC>& cp = this->GetCacheProcessor(dir);
	SPoint remoteloc = cp.LocalToRemote(location);
	isInANeighborsShared |= ! IsInCache(remoteloc); //all tiles same size
      }
    return isInANeighborsShared;
  }

  template <class EC>
  bool Tile<EC>::IsLiveSite(const SPoint & location) const
  {
    if (!IsInTile(location))
    {
      return false;
    }
    if (IsOwnedSite(location))
    {
      return true;
    }

    // In-tile + not-owned => in-cache
    return IsCacheSitePossibleEventCenter(location);
  }

  template <class EC>
  bool Tile<EC>::IsInUncachedTile(const SPoint& pt) const
  {
    //return ((u32) pt.GetX()) < OWNED_WIDTH && ((u32) pt.GetY() < OWNED_HEIGHT);
    return (pt.GetX() < (s32) OWNED_WIDTH) && (pt.GetY() < (s32) OWNED_HEIGHT);
  }

  template <class EC>
  typename Tile<EC>::Region Tile<EC>::RegionFromIndex(const u32 index, const u32 tileSide)
  {
    MFM_API_ASSERT_ARG(index < tileSide);

    enum { R = EVENT_WINDOW_RADIUS };

    const u32 hiddenDim = tileSide - R * 6;

    if(index < R * REGION_HIDDEN)
    {
      return (Region)(index / R);
    }
    else if(index >= R * REGION_HIDDEN + hiddenDim)
    {
      return (Region)((tileSide - index - 1) / R);
    }
    else
    {
      return REGION_HIDDEN;
    }
  }


  template <class EC>
  typename Tile<EC>::Region Tile<EC>::RegionIn(const SPoint& pt)
  {
    //return MIN(RegionFromIndex((u32)pt.GetX(), TILE_WIDTH),
    //           RegionFromIndex((u32)pt.GetY(), TILE_HEIGHT));
    return MIN(RegionFromIndex(pt.GetX(), (s32) TILE_WIDTH),
               RegionFromIndex(pt.GetY(), (s32) TILE_HEIGHT));
  }

  template <class EC>
  bool Tile<EC>::Advance()
  {
    if (!ConsiderStateChange())
    {
      return false;
    }

    bool didWork = false;
    State curState = GetCurrentState();
    if (!m_enabled)
    {
      curState = PASSIVE;
    }

    switch (curState)
    {
    case OFF:
      break;
    case ACTIVE:
      didWork |= AdvanceComputation();
      MFM_LOG_DBG6(("Tile %s: AdvanceComputation->%d",
                    this->GetLabel(),
                    didWork));
      // FALL THROUGH
    case PASSIVE:
      didWork |= AdvanceCommunication();
      break;
    default:
      FAIL(ILLEGAL_STATE);
    }
    return didWork;
  }

  template <class EC>
  bool Tile<EC>::AllCacheProcessorsIdle()
  {
    for (u32 i = 0; i < Dirs::DIR_COUNT; ++i)  // Can doing this in order create bias??
    {
      if (!m_cacheProcessors[i].IsIdle() || !m_cacheProcessors[i].IsUnclaimed())
      {
        return false;
      }
    }
    MFM_LOG_DBG6(("Tile %s All CPs idle", this->GetLabel()));
    return true;
  }

  template <class EC>
  void Tile<EC>::SetBackgroundRadiationEnabled(bool on)
  {
    m_backgroundRadiationEnabled = on;
  }

  template <class EC>
  void Tile<EC>::SetForegroundRadiationEnabled(bool on)
  {
    m_foregroundRadiationEnabled = on;
  }

  template <class EC>
  void Tile<EC>::SetRequestedState(State state)
  {
    Mutex::ScopeLock lock(m_stateAccess);
    MFM_LOG_DBG4(("Requesting state %s for Tile %s (current: %s)",
		  GetStateName(state),
		  this->GetLabel(),
		  GetStateName(m_state)));
    m_requestedState = state;
  }

  template <class EC>
  bool Tile<EC>::ConsiderStateChange()
  {
    Mutex::ScopeLock lock(m_stateAccess);

    if (m_state == m_requestedState)
    {
      return true;
    }

#if 0 // Mon Jun 29 04:45:42 2015 XXX enabled means ONLY do or don't ORIGINATE events.. all other processing continues
    if (!m_enabled)
    {
      return false;
    }
#endif

    switch (m_requestedState)
    {
    case ACTIVE:
      if (m_state == OFF || m_state == PASSIVE)
      {
        m_state = m_requestedState;
        return true;
      }

      if (AllCacheProcessorsIdle())
      {
        m_state = m_requestedState;
        return true;
      }
      return false;

    case PASSIVE:
      m_state = m_requestedState;
      return true;

    case OFF:
      FAIL(INCOMPLETE_CODE);
    }
    return true;
  }

  template <class EC>
  bool Tile<EC>::AdvanceComputation()
  {

    //NON_ACTIVE,
    if (!IsActive())
    {
      return false;
    }

    if (!IsEnabled())
    {
      SleepMsec(2);
      return false;
    }

    //INITIATE_EVENT,
    SPoint pt = GetRandomOwnedCoord(); //adjusted to range (0..Tile_Width, 0...Tile_Height)
    if (RegionIn(pt) == REGION_CACHE)
      FAIL(ILLEGAL_STATE);

    return m_window.TryEventAt(pt);
  }

  template <class EC>
  bool Tile<EC>::AdvanceCommunication()
  {
    bool didWork = false;
    for (m_dirIterator.ShuffleOrReset(m_random); m_dirIterator.HasNext(); )
    {
      u32 i = m_dirIterator.Next();
      MFM_API_ASSERT_STATE(Dirs::IsValidDir(i, IsTileGridLayoutStaggered()));
      CacheProcessor<EC> & cp = m_cacheProcessors[i];
      if(cp.IsConnected())
	didWork |= cp.Advance();
    }
    return didWork;
  }

  template <class EC>
  void Tile<EC>::ReportTileStatus(Logger::Level level)
  {
    LOG.Log(level," ===TILE %s STATUS REPORT: cur %s req %s===",
            m_label.GetZString(),
            GetStateName(GetCurrentState()),
            GetStateName(m_requestedState));

    LOG.Log(level,"  ==Tile %s Global==", m_label.GetZString());
    LOG.Log(level,"   Address: %p", (void*) this);
    LOG.Log(level,"   Error stack top: %p", (void*) m_errorEnvironmentStackTop);

    LOG.Log(level,"  ==Tile %s Atomic==", m_label.GetZString());
    LOG.Log(level,"   Recount needed: %s", m_cdata.m_needRecount?"true":"false");

    LOG.Log(level,"  ==Tile %s Events==", m_label.GetZString());
    LOG.Log(level,"   Events: %dM (total)", (u32) (GetEventsExecuted() / 1000000));

    for (u32 d = Dirs::NORTH; d <= Dirs::NORTHWEST; ++d)
    {
      CacheProcessor<EC> & cp = GetCacheProcessor(d);
      cp.ReportCacheProcessorStatus(level);
    }
  }
} /* namespace MFM */
