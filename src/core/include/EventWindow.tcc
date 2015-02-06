/* -*- C++ -*- */
#include "MDist.h"
#include "Element.h"
#include "Element_Empty.h"
#include "Tile.h"
#include "ChannelEnd.h"
#include "PacketIO.h"

namespace MFM {

  template <class EC>
  bool EventWindow<EC>::TryEventAt(const SPoint & tcenter)
  {
    LOG.Debug2("EW::TryEventAt(%d,%d)",
               tcenter.GetX(),
               tcenter.GetY());
    ++m_eventWindowsAttempted;

    if (RejectOnRecency(tcenter))
    {
      return false;
    }

    if (!InitForEvent(tcenter))
    {
      return false;
    }

    RecordEventAtTileCoord(tcenter);
    ExecuteEvent();

    return true;
  }

  template <class EC>
  void EventWindow<EC>::RecordEventAtTileCoord(const SPoint tcoord)
  {
    ++m_eventWindowsExecuted;
    Tile<EC> & t = GetTile();
    SPoint owned = Tile<EC>::TileCoordToOwned(tcoord);
    t.m_lastEventCenterOwned = owned;
    t.GetSite(owned).SetLastEventEventNumber(m_eventWindowsExecuted);
  }

  template <class EC>
  bool EventWindow<EC>::RejectOnRecency(const SPoint tcoord)
  {
    Tile<EC> & t = GetTile();
    const u32 warpFactor = t.GetWarpFactor();
    SPoint owned = Tile<EC>::TileCoordToOwned(tcoord);
    u32 eventAge = t.GetUncachedEventAge(owned);
    return !GetRandom().OddsOf(eventAge + warpFactor*t.GetSites(), 10*t.GetSites());
  }

  template <class EC>
  void EventWindow<EC>::ExecuteEvent()
  {
    LOG.Debug2("EW::ExecuteEvent");
    if (m_ewState != COMPUTE)
    {
      FAIL(ILLEGAL_STATE);
    }

    ExecuteBehavior();

    InitiateCommunications();
  }

  template <class EC>
  void EventWindow<EC>::InitiateCommunications()
  {
    LOG.Debug2("EW::InitiateCommunications");
    if (m_ewState != COMPUTE)
    {
      FAIL(ILLEGAL_STATE);
    }

    // Step 1: Write back the event window and set up the CacheProcessors
    StoreToTile();

    // Step 2: We are done!
    m_ewState = FREE;
  }

  template <class EC>
  void EventWindow<EC>::ExecuteBehavior()
  {
    LOG.Debug2("EW::ExecuteBehavior");
    Tile<EC> & t = GetTile();
    unwind_protect(
    {
      if(!GetCenterAtomDirect().IsSane())
      {
        LOG.Debug("FE(INSANE)");
      }
      else
      {
        LOG.Debug("FE(%x) (SANE)",GetCenterAtomDirect().GetType());
      }

      SetCenterAtomDirect(t.GetEmptyAtom());
    },
    {
      LOG.Debug2("ET::Execute");
      t.GetElementTable().Execute(*this);
    });
  }

  template <class EC>
  void EventWindow<EC>::Diffuse()
  {
    EventWindow & window = *this;
    Random & random = window.GetRandom();
    Tile<EC>& tile = window.GetTile();
    const MDist<R> & md = MDist<R>::get();

    SPoint sp;
    md.FillRandomSingleDir(sp, random);

    // Don't diffuse stuff into the great nowhere, but consider
    // 'bouncing' off the edge of the universe
    if (!window.IsLiveSiteDirect(sp)) {
      sp *= -1;
      if (!window.IsLiveSiteDirect(sp))  // Wow this is a tight universe!
        return;
    }

    T us = window.GetCenterAtomDirect();
    T other = window.GetRelativeAtomDirect(sp);
    const Element<EC> * ourElt = tile.GetElement(us.GetType());
    const Element<EC> * elt = tile.GetElement(other.GetType());

    if (!other.IsSane() || !(elt = tile.GetElement(other.GetType())))
      return;       // Any confusion, let the engine sort it out first

    u32 thisWeight = elt->Diffusability(window, sp, SPoint(0,0));
    u32 ourWeight = ourElt->Diffusability(window, SPoint(0,0), sp);
    if (random.OddsOf(MIN(ourWeight, thisWeight), COMPLETE_DIFFUSABILITY))
      window.SwapAtomsDirect(sp, SPoint(0, 0));
  }

  template <class EC>
  bool EventWindow<EC>::InitForEvent(const SPoint & center)
  {
    LOG.Debug2("EW::InitForEvent(%d,%d)",center.GetX(),center.GetY());

    if (!IsFree())
    {
      FAIL(ILLEGAL_STATE);  // Don't be callin' when I'm not free
    }

    if (!AcquireAllLocks(center))
    {
      LOG.Debug2("EW::InitForEvent - abandoned");
      return false;
    }

    m_center = center;
    m_ewState = COMPUTE;
    m_sym = PSYM_NORMAL;

    LoadFromTile();
    return true;
  }

  template <class EC>
  bool EventWindow<EC>::AcquireRegionLocks()
  {
    LOG.Debug2("EW::AcquireRegionLocks");
    // We cannot still have any cacheprocessors in use
    for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
    {
      if (m_cacheProcessorsLocked[i] != 0)
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    if (((s32) m_lockRegion) == -1)
    {
      LOG.Debug2("EW::AcquireRegionLocks - none needed");
      return true;  // Nobody is needed
    }

    // At least one lock may be needed

    Tile<EC> & tile = GetTile();
    Dir baseDir = m_lockRegion;
    Dir stopDir = Dirs::CWDir(baseDir);
    u32 needed = 1;
    if (Dirs::IsCorner(m_lockRegion))
    {
      baseDir = Dirs::CCWDir(baseDir);
      stopDir = Dirs::CWDir(stopDir);
      needed = 3;
    }

    LOG.Debug2("EW::AcquireRegionLocks - checking %d", needed);

    u32 got = 0;
    for (Dir dir = baseDir; dir != stopDir; dir = Dirs::CWDir(dir))
    {
      CacheProcessor<EC> & cp = tile.GetCacheProcessor(dir);

      if (!cp.IsConnected())
      {
        // Whups, didn't really need that one.  Leave it null, since
        // the other loops check all MAX_CACHES_TO_UPDATE slots anyway
        LOG.Debug2("EW::AcquireRegionLocks - skip: %s unconnected",
                  Dirs::GetName(dir));
        --needed;
        continue;
      }

      if (!cp.IsIdle())
      {
        LOG.Debug2("EW::AcquireRegionLocks - fail: %s cp not idle",
                  Dirs::GetName(dir));
        break;  // Already otherwise engaged
      }

      bool locked = cp.TryLock(m_lockRegion);
      if (!locked)
      {
        LOG.Debug2("EW::AcquireRegionLocks - fail: didn't get %s lock",
                  Dirs::GetName(dir));
        break; // Didn't get the lock
      }
      LOG.Debug2("EW::AcquireRegionLocks #%d, %s locked",
                got,
                Dirs::GetName(dir));
      m_cacheProcessorsLocked[got] = &cp;
      ++got;
    }

    if (got < needed)
    {
      LOG.Debug2("EW::AcquireRegionLocks - got %d but needed %d", got, needed);
      // Opps, didn't get all, free any we got
      for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
      {
        if (m_cacheProcessorsLocked[i])
        {
          CacheProcessor<EC> & cp = *m_cacheProcessorsLocked[i];
          cp.Unlock();
          LOG.Debug2("EW::AcquireRegionLocks #%d freed", i);
          m_cacheProcessorsLocked[i] = 0;
        }
      }

      return false;
    }

    // Activate everything we got
    for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
    {
      if (m_cacheProcessorsLocked[i])
      {
        LOG.Debug2("EW::AcquireRegionLocks activate #%d", i);
        m_cacheProcessorsLocked[i]->Activate();
      }
    }

    return true;
  }

  template <class EC>
  bool EventWindow<EC>::AcquireAllLocks(const SPoint& tileCenter)
  {
    Tile<EC> & t = GetTile();
    m_lockRegion = t.GetLockDirection(tileCenter);
    return AcquireRegionLocks();
  }

  template <class EC>
  EventWindow<EC>::EventWindow(Tile<EC> & tile)
    : m_tile(tile)
    , m_eventWindowsAttempted(0)
    , m_eventWindowsExecuted(0)
    , m_center(0,0)
    , m_lockRegion(-1)
    , m_sym(PSYM_NORMAL)
    , m_ewState(FREE)
  {
    for (u32 i = 0; i < SITE_COUNT; ++i)
    {
      m_isLiveSite[i] = false;
    }

    for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
    {
      m_cacheProcessorsLocked[i] = 0;
    }

  }

  template <class EC>
  SPoint EventWindow<EC>::MapToTileSymValid(const SPoint& offset) const
  {
    if (!InWindow(offset)) FAIL(ILLEGAL_ARGUMENT);
    return MapToTile(offset);
  }

  template <class EC>
  u32 EventWindow<EC>::MapToIndexSymValid(const SPoint & loc) const
  {
    return MapToIndexDirectValid(SymMap(loc, m_sym, SPoint(2*R, 2*R)));
  }

  template <class EC>
  u32 EventWindow<EC>::MapToIndexDirectValid(const SPoint & loc) const
  {
    const MDist<R> & md = MDist<R>::get();
    s32 index = md.FromPoint(loc,R);
    if (index < 0)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return (u32) index;
  }

  template <class EC>
  void EventWindow<EC>::LoadFromTile()
  {
    const MDist<R> & md = MDist<R>::get();
    Tile<EC> & tile = GetTile();
    for (u32 i = 0; i < SITE_COUNT; ++i)
    {
      const SPoint & pt = md.GetPoint(i) + m_center;
      m_atomBuffer[i] = *tile.GetAtom(pt);
      m_isLiveSite[i] = tile.IsLiveSite(pt);
    }
  }

  template <class EC>
  void EventWindow<EC>::StoreToTile()
  {
    LOG.Debug2("EW::StoreToTile");

    // First initialize the cache processors
    u32 maxCacheUsed = 0;
    for (u32 i = 0;i < MAX_CACHES_TO_UPDATE; ++i)
    {
      if (m_cacheProcessorsLocked[i])
      {
        maxCacheUsed = i + 1;
        m_cacheProcessorsLocked[i]->StartLoading(m_center);
      }
    }

    // Now write back changes and notify the cps
    const MDist<R> & md = MDist<R>::get();
    Tile<EC> & tile = GetTile();

    for (u32 i = 0; i < SITE_COUNT; ++i)
    {
      const SPoint & pt = md.GetPoint(i) + m_center;
      bool dirty = false;
      if (m_isLiveSite[i])
      {
        if (m_atomBuffer[i] != *tile.GetAtom(pt))
        {
          tile.PlaceAtom(m_atomBuffer[i], pt);
          dirty = true;
        }

        // Let the CPs see even some unchanged atoms, for spot checks
        for (u32 j = 0; j < maxCacheUsed; ++j)
        {
          if (m_cacheProcessorsLocked[j] != 0)
          {
            m_cacheProcessorsLocked[j]->MaybeSendAtom( *tile.GetAtom(pt), dirty, i);
          }
        }
      }
    }

    LOG.Debug2("EW::StoreToTile releasing up to %d", maxCacheUsed);
    // Finally, release the cache processors to take it from here
    for (u32 i = 0;i < maxCacheUsed; ++i)
    {
      if (m_cacheProcessorsLocked[i])
      {
        m_cacheProcessorsLocked[i]->StartShipping();
        m_cacheProcessorsLocked[i] = 0;
      }
    }
  }

  template <class EC>
  bool EventWindow<EC>::SetRelativeAtomSym(const SPoint& offset, const T & atom)
  {
    u32 idx = MapToIndexSymValid(offset);
    if (m_isLiveSite[idx])
    {
      m_atomBuffer[idx] = atom;
      return true;
    }
    return false;
  }

  template <class EC>
  bool EventWindow<EC>::SetRelativeAtomDirect(const SPoint& offset, const T & atom)
  {
    u32 idx = MapToIndexDirectValid(offset);
    if (m_isLiveSite[idx])
    {
      m_atomBuffer[idx] = atom;
      return true;
    }
    return false;
  }

  template <class EC>
  const typename EC::ATOM_CONFIG::ATOM_TYPE& EventWindow<EC>::GetRelativeAtomSym(const SPoint& offset) const
  {
    return m_atomBuffer[MapToIndexSymValid(offset)];
  }

  template <class EC>
  const typename EC::ATOM_CONFIG::ATOM_TYPE& EventWindow<EC>::GetRelativeAtomDirect(const SPoint& offset) const
  {
    return m_atomBuffer[MapToIndexDirectValid(offset)];
  }

  template <class EC>
  const typename EC::ATOM_CONFIG::ATOM_TYPE& EventWindow<EC>::GetRelativeAtomSym(const Dir mooreOffset) const
  {
    SPoint pt;
    Dirs::FillDir(pt, mooreOffset);
    return GetRelativeAtomSym(pt);
  }

  template <class EC>
  const typename EC::ATOM_CONFIG::ATOM_TYPE& EventWindow<EC>::GetRelativeAtomDirect(const Dir mooreOffset) const
  {
    SPoint pt;
    Dirs::FillDir(pt, mooreOffset);
    return GetRelativeAtomDirect(pt);
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsSym(const u32 syma, const u32 symb)
  {
    u32 idxa = MapIndexToIndexSymValid(syma);
    u32 idxb = MapIndexToIndexSymValid(symb);

    T tmp = m_atomBuffer[idxa];
    m_atomBuffer[idxa] = m_atomBuffer[idxb];
    m_atomBuffer[idxb] = tmp;
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsDirect(const u32 idxa, const u32 idxb)
  {
    T tmp = m_atomBuffer[idxa];
    m_atomBuffer[idxa] = m_atomBuffer[idxb];
    m_atomBuffer[idxb] = tmp;
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsSym(const SPoint& locA, const SPoint& locB)
  {
    u32 idxa = MapToIndexSymValid(locA);
    u32 idxb = MapToIndexSymValid(locB);

    T tmp = m_atomBuffer[idxa];
    m_atomBuffer[idxa] = m_atomBuffer[idxb];
    m_atomBuffer[idxb] = tmp;
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsDirect(const SPoint& locA, const SPoint& locB)
  {
    u32 idxa = MapToIndexDirectValid(locA);
    u32 idxb = MapToIndexDirectValid(locB);

    T tmp = m_atomBuffer[idxa];
    m_atomBuffer[idxa] = m_atomBuffer[idxb];
    m_atomBuffer[idxb] = tmp;
  }

} /* namespace MFM */
