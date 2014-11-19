/* -*- C++ -*- */
#include "MDist.h"
#include "Element_Empty.h"
#include "Logger.h"
#include "AtomSerializer.h"
#include "Util.h"

namespace MFM
{
  template <class CC>
  Tile<CC>::Tile()
    : m_cdata(*this)
    , m_window(*this)
    , m_state(OFF)
    , m_enabled(true)
    , m_backgroundRadiation(false)
    , m_requestedState(OFF)
    , m_warpFactor(3)
  {
    Init();
  }

  template <class CC>
  void Tile<CC>::Init()
  {
    m_elementTable.Reinit();

    Element_Empty<CC>::THE_INSTANCE.AllocateType();
    RegisterElement(Element_Empty<CC>::THE_INSTANCE);

    ClearAtoms();

  }

  template <class CC>
  typename CC::ATOM_TYPE Tile<CC>::GetEmptyAtom() const
  {
    return Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
  }

  template <class CC>
  void Tile<CC>::XRay(u32 siteOdds, u32 bitOdds)
  {
    Random & random = GetRandom();
    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
        if (random.OneIn(siteOdds))
        {
          m_atoms[x][y].XRay(random, bitOdds);
        }
      }
    }
  }

  template <class CC>
  void Tile<CC>::ClearAtoms()
  {
    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
        m_atoms[x][y] = Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
      }
    }
    NeedAtomRecount();

    for(u32 x = 0; x < OWNED_SIDE; x++)
    {
      for(u32 y = 0; y < OWNED_SIDE; y++)
      {
        m_lastChangedEventNumber[x][y] = 0;

        // Set last event deep in the past to avoid initial color
        m_lastEventEventNumber[x][y] = -1000*GetSites();
      }
    }

  }

  template <class CC>
  void Tile<CC>::Connect(AbstractChannel& channel, LonglivedLock & lock, Dir toCache)
  {
    CacheProcessor<CC> & cxn = GetCacheProcessor(toCache);

    if (cxn.IsConnected())
    {
      FAIL(ILLEGAL_STATE);
    }

    cxn.ClaimCacheProcessor(*this, channel, lock, toCache);
  }

  template <class CC>
  CacheProcessor<CC> & Tile<CC>::GetCacheProcessor(Dir toCache)
  {
    return const_cast<CacheProcessor<CC>&>(static_cast<const Tile<CC>*>(this)->GetCacheProcessor(toCache));
  }

  template <class CC>
  const CacheProcessor<CC> & Tile<CC>::GetCacheProcessor(Dir toCache) const
  {
    if (!Dirs::IsLegalDir(toCache))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

    return m_cacheProcessors[toCache];
  }

  template <class CC>
  Random& Tile<CC>::GetRandom()
  {
    return m_random;
  }

  template <class CC>
  s32 Tile<CC>::CountData::GetAtomCount(u32 type)
  {
    s32 idx = m_tile.m_elementTable.GetIndex(type);
    if (idx < 0)
    {
      return -1;
    }

    RecountIfNeeded();

    return m_atomCount[idx];
  }

  template <class CC>
  void Tile<CC>::CountData::RecountAtoms()
  {
    for(u32 i = 0; i < ELEMENT_TABLE_SIZE; i++)
    {
      m_atomCount[i] = 0;
    }

    m_illegalAtomCount = 0;

    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
        const SPoint pt(x,y);

        if (m_tile.IsInCache(pt))
        {
          continue;
        }

        u32 atype = m_tile.m_atoms[x][y].GetType();
        s32 idx = m_tile.m_elementTable.GetIndex(atype);

        if (idx < 0)
        {
          ++m_illegalAtomCount;
        }
        else
        {
          ++m_atomCount[idx];
        }
      }
    }
  }

  template <class CC>
  u32 Tile<CC>::GetUncachedWriteAge(const SPoint site) const
  {
    if (!IsInUncachedTile(site))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return (u32) (GetEventsExecuted() -
                  m_lastChangedEventNumber[site.GetX()][site.GetY()]);
  }

  template <class CC>
  u32 Tile<CC>::GetUncachedEventAge(const SPoint site) const
  {
    if (!IsInUncachedTile(site))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return (u32) (GetEventsExecuted() -
                  m_lastEventEventNumber[site.GetX()][site.GetY()]);
  }

  template <class CC>
  void Tile<CC>::InternalPutAtom(const T & atom, s32 x, s32 y)
  {
    if (((u32) x) >= TILE_WIDTH || ((u32) y) >= TILE_WIDTH)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    }
    m_atoms[x][y] = atom;
  }

  template <class CC>
  template <u32 REACH>
  Dir Tile<CC>::RegionAt(const SPoint& sp) const
  {
    UPoint pt = MakeUnsigned(sp);

    if(pt.GetX() < REACH)
    {
      if(pt.GetY() < REACH)
      {
        return Dirs::NORTHWEST;
      }
      else if(pt.GetY() >= TILE_WIDTH - REACH)
      {
        return Dirs::SOUTHWEST;
      }
      return Dirs::WEST;
    }
    else if(pt.GetX() >= TILE_WIDTH - REACH)
    {
      if(pt.GetY() < REACH)
      {
        return Dirs::NORTHEAST;
      }
      else if(pt.GetY() >= TILE_WIDTH - REACH)
      {
        return Dirs::SOUTHEAST;
      }
      return Dirs::EAST;
    }

    if(pt.GetY() < REACH)
    {
      return Dirs::NORTH;
    }
    else if(pt.GetY() >= TILE_WIDTH - REACH)
    {
      return Dirs::SOUTH;
    }

    return (Dir)-1;
  }

  template <class CC>
  Dir Tile<CC>::CacheAt(const SPoint& pt) const
  {
    return RegionAt<EVENT_WINDOW_RADIUS>(pt);
  }

  template <class CC>
  Dir Tile<CC>::SharedAt(const SPoint& pt) const
  {
    return RegionAt<EVENT_WINDOW_RADIUS * 2>(pt);
  }

  template <class CC>
  Dir Tile<CC>::VisibleAt(const SPoint& pt) const
  {
    return RegionAt<EVENT_WINDOW_RADIUS * 3>(pt);
  }

  template <class CC>
  bool Tile<CC>::ApplyCacheUpdate(bool isDifferent, const T& atom, const SPoint& site)
  {
    if (IsInHidden(site))  // That would make no sense
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

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

  template <class CC>
  void Tile<CC>::PlaceAtom(const T& atom, const SPoint& pt)
  {
    if (!IsLiveSite(pt))
    {
      if (atom.GetType() != Element_Empty<CC>::THE_INSTANCE.GetType())
      {
        LOG.Debug("Not placing type %04x at (%2d,%2d) of %s",
                  atom.GetType(), pt.GetX(), pt.GetY(), this->GetLabel());
      }
      return;
    }

    T newAtom = atom;
    unwind_protect(
    {
      InternalPutAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(),
                      pt.GetX(), pt.GetY());
      LOG.Warning("Failure during PlaceAtom, erased (%2d,%2d) of %s",
                  pt.GetX(), pt.GetY(), this->GetLabel());
    },
    {
      if(m_backgroundRadiation &&
         m_random.OneIn(BACKGROUND_RADIATION_SITE_ODDS))
      {
        // Write fault!
        newAtom.XRay(m_random, BACKGROUND_RADIATION_BIT_ODDS);
      }

      const T& oldAtom = *GetAtom(pt);
      bool owned = IsOwnedSite(pt);
      if (oldAtom != newAtom)
      {
        NeedAtomRecount();
        if (owned)
        {
          const SPoint opt = TileCoordToOwned(pt);
          m_lastChangedEventNumber[opt.GetX()][opt.GetY()] = GetEventsExecuted();
        }

        InternalPutAtom(newAtom,pt.GetX(),pt.GetY());
      }
    });
  }

  template <class CC>
  bool Tile<CC>::IsConnected(Dir dir) const
  {
    const CacheProcessor<CC> & cxn = GetCacheProcessor(dir);
    return cxn.IsConnected();
  }

  template <class CC>
  bool Tile<CC>::IsReachableViaCacheProtocol(const SPoint & location) const
  {
    if (!IsInShared(location))
    {
      return false;
    }

    Dir dir = SharedAt(location);
    if (IsConnected(dir))
    {
      return true;
    }

    if (Dirs::IsCorner(dir))
    {
      if (IsConnected(Dirs::CCWDir(dir)))
      {
        return true;
      }
      if (IsConnected(Dirs::CWDir(dir)))
      {
        return true;
      }
    }
    return false;
  }

  template <class CC>
  bool Tile<CC>::IsCacheSitePossibleEventCenter(const SPoint & location) const
  {
    if (!IsInCache(location))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

    return IsConnected(CacheAt(location));
  }

  template <class CC>
  bool Tile<CC>::IsLiveSite(const SPoint & location) const
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

  template <class CC>
  bool Tile<CC>::IsInUncachedTile(const SPoint& pt)
  {
    return ((u32) pt.GetX()) < OWNED_SIDE && ((u32) pt.GetY() < OWNED_SIDE);
  }

  template <class CC>
  bool Tile<CC>::HasAnyConnections(Dir regionDir) const
  {
    switch(regionDir)
    {
    case Dirs::NORTH:
    case Dirs::EAST:
    case Dirs::SOUTH:
    case Dirs::WEST:
      return IsConnected(regionDir);

    case Dirs::NORTHWEST:
    case Dirs::NORTHEAST:
    case Dirs::SOUTHEAST:
    case Dirs::SOUTHWEST:
      if (IsConnected(regionDir)) return true;
      if (IsConnected(Dirs::CCWDir(regionDir))) return true;
      if (IsConnected(Dirs::CWDir(regionDir))) return true;
      return false;

    default:
      FAIL(ILLEGAL_ARGUMENT);
    }
  }

  template <class CC>
  TileRegion Tile<CC>::RegionFromIndex(const u32 index)
  {
    // Extract short names for parameter types
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

    if(index > P::TILE_WIDTH)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS); /* Index out of Tile bounds */
    }

    const u32 hiddenWidth = P::TILE_WIDTH - R * 6;

    if(index < R * REGION_HIDDEN)
    {
      return (TileRegion)(index / R);
    }
    else if(index >= R * REGION_HIDDEN + hiddenWidth)
    {
      return (TileRegion)((index - (R * REGION_HIDDEN) - hiddenWidth) / R);
    }
    else
    {
      return REGION_HIDDEN;
    }
  }

  template <class CC>
  TileRegion Tile<CC>::RegionIn(const SPoint& pt)
  {
    return MIN(RegionFromIndex((u32)pt.GetX()),
               RegionFromIndex((u32)pt.GetY()));
  }

  template <class CC>
  bool Tile<CC>::Advance()
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

  template <class CC>
  bool Tile<CC>::AllCacheProcessorsIdle()
  {
    for (u32 d = 0; d < Dirs::DIR_COUNT; ++d)
    {
      if (!m_cacheProcessors[d].IsIdle())
      {
        return false;
      }
    }
    MFM_LOG_DBG6(("Tile %s All CPs idle",
                  this->GetLabel()));
    return true;
  }

  template <class CC>
  void Tile<CC>::SetBackgroundRadiation(bool on)
  {
    m_backgroundRadiation = on;
  }

  template <class CC>
  void Tile<CC>::SetRequestedState(State state)
  {
    Mutex::ScopeLock lock(m_stateAccess);
    LOG.Debug("Requesting state %s for Tile %s",
              GetStateName(state),
              this->GetLabel());
    m_requestedState = state;
  }

  template <class CC>
  bool Tile<CC>::ConsiderStateChange()
  {
    Mutex::ScopeLock lock(m_stateAccess);

    if (m_state == m_requestedState)
    {
      return true;
    }

    if (!m_enabled)
    {
      return false;
    }

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

  template <class CC>
  bool Tile<CC>::AdvanceComputation()
  {

    //NON_ACTIVE,
    if (!IsActive())
    {
      return false;
    }

    //INITIATE_EVENT,
    SPoint pt = GetRandomOwnedCoord();
    return m_window.TryEventAt(pt);
  }

  template <class CC>
  bool Tile<CC>::AdvanceCommunication()
  {
    bool didWork = false;
    for (u32 i = 0; i < Dirs::DIR_COUNT; ++i)
    {
      CacheProcessor<CC> & cp = m_cacheProcessors[i];
      didWork |= cp.Advance();
    }
    return didWork;
  }

  template <class CC>
  void Tile<CC>::ReportTileStatus(Logger::Level level)
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
      CacheProcessor<CC> & cp = GetCacheProcessor(d);
      cp.ReportCacheProcessorStatus(level);
    }
  }
} /* namespace MFM */
