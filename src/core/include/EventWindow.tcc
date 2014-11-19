/* -*- C++ -*- */
#include "MDist.h"
#include "Element.h"
#include "Element_Empty.h"
#include "Tile.h"
#include "ChannelEnd.h"
#include "PacketIO.h"

namespace MFM {

  template <class CC>
  bool EventWindow<CC>::TryEventAt(const SPoint & center)
  {
    LOG.Debug("EW::TryEventAt(%d,%d)",
              center.GetX(),
              center.GetY());
    ++m_eventWindowsAttempted;
    if (!InitForEvent(center))
    {
      return false;
    }

    ++m_eventWindowsExecuted;
    ExecuteEvent();

    return true;
  }

  template <class CC>
  void EventWindow<CC>::ExecuteEvent()
  {
    LOG.Debug("EW::ExecuteEvent");
    if (m_ewState != COMPUTE)
    {
      FAIL(ILLEGAL_STATE);
    }

    ExecuteBehavior();

    InitiateCommunications();
  }

  template <class CC>
  void EventWindow<CC>::InitiateCommunications()
  {
    LOG.Debug("EW::InitiateCommunications");
    if (m_ewState != COMPUTE)
    {
      FAIL(ILLEGAL_STATE);
    }

    // Step 1: Write back the event window and set up the CacheProcessors
    StoreToTile();

    // Step 2: We are done!
    m_ewState = FREE;
  }

  template <class CC>
  void EventWindow<CC>::ExecuteBehavior()
  {
    LOG.Debug("EW::ExecuteBehavior");
    Tile<CC> & t = GetTile();
    unwind_protect(
    {
      if(!GetCenterAtom().IsSane())
      {
        LOG.Debug("FE(INSANE)");
      }
      else
      {
        LOG.Debug("FE(%x) (SANE)",GetCenterAtom().GetType());
      }

      SetCenterAtom(t.GetEmptyAtom());
    },
    {
      LOG.Debug("ET::Execute");
      t.GetElementTable().Execute(*this);
    });
  }

  template <class CC>
  void EventWindow<CC>::Diffuse()
  {
    EventWindow & window = *this;
    Random & random = window.GetRandom();
    Tile<CC>& tile = window.GetTile();
    const MDist<R> & md = MDist<R>::get();

    SPoint sp;
    md.FillRandomSingleDir(sp, random);

    // Don't diffuse stuff into the great nowhere, but consider
    // 'bouncing' off the edge of the universe
    if (!window.IsLiveSite(sp)) {
      sp *= -1;
      if (!window.IsLiveSite(sp))  // Wow this is a tight universe!
        return;
    }

    T us = window.GetCenterAtom();
    T other = window.GetRelativeAtom(sp);
    const Element<CC> * ourElt = tile.GetElement(us.GetType());
    const Element<CC> * elt = tile.GetElement(other.GetType());

    if (!other.IsSane() || !(elt = tile.GetElement(other.GetType())))
      return;       // Any confusion, let the engine sort it out first

    u32 thisWeight = elt->Diffusability(window, sp, SPoint(0,0));
    u32 ourWeight = ourElt->Diffusability(window, SPoint(0,0), sp);
    if (random.OddsOf(MIN(ourWeight, thisWeight), Element<CC>::COMPLETE_DIFFUSABILITY))
      window.SwapAtoms(sp, SPoint(0, 0));
  }

  template <class CC>
  bool EventWindow<CC>::InitForEvent(const SPoint & center)
  {
    LOG.Debug("EW::InitForEvent(%d,%d)",center.GetX(),center.GetY());

    if (!IsFree())
    {
      FAIL(ILLEGAL_STATE);  // Don't be callin' when I'm not free
    }

    if (!AcquireAllLocks(center))
    {
      LOG.Debug("EW::InitForEvent - abandoned");
      return false;
    }

    m_center = center;
    m_ewState = COMPUTE;
    m_sym = PSYM_NORMAL;

    LoadFromTile();
    return true;
  }

#if 0
  template <class CC>
  bool EventWindow<CC>::Advance()
  {
    switch (m_ewState)
    {
    case COMPUTE:
      // COMPUTE finishes atomically; can't be here
    default: FAIL(ILLEGAL_STATE);
    case FREE:
      return false;

    case COMMUNICATE:
      {
        return SendCacheUpdates();
      }
    }
  }
#endif

  template <class CC>
  bool EventWindow<CC>::SendCacheUpdates()
  {
    FAIL(INCOMPLETE_CODE);  // XXX REIMPLEMENT|DITCH ME
#if 0
    Tile<CC> & t = GetTile();
    Random & random = t.GetRandom();
    PacketIO pbuffer;
    bool didWork = false;
    for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
    {
      CacheUpdateInfo & cui = m_cacheInfo[i];
      if (cui.m_lockedTo == (Dir) -1)
      {
        continue;
      }
      ChannelEnd & cxn = t.GetChannelEnd(cui.m_lockedTo);
      const u32 checkOdds = cxn.GetCheckOdds();
      while (cui.m_atomsConsidered < SITE_COUNT)
      {
        bool dirty = m_isDirtySite[cui.m_atomsConsidered];
        if (dirty || random.OneIn(checkOdds))
        {
          if (!pbuffer.SendAtom<CC>(dirty ? PACKET_UPDATE : PACKET_CHECK,
                                    cxn, cui.m_atomsConsidered,
                                    m_atomBuffer[cui.m_atomsConsidered]))
          {
            break; // No room.  Try again later; don't advance m_atomsConsidered
          }
          else
          {
            didWork |= true;
          }
        }
        ++cui.m_atomsConsidered;
      }
    }
    return didWork;
#endif
  }

  template <class CC>
  bool EventWindow<CC>::ReceiveCacheReplies()
  {
    FAIL(INCOMPLETE_CODE);
  }

  template <class CC>
  bool EventWindow<CC>::AcquireRegionLocks()
  {
    LOG.Debug("EW::AcquireRegionLocks");
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
      LOG.Debug("EW::AcquireRegionLocks - none needed");
      return true;  // Nobody is needed
    }

    s32 needed = 1;
    s32 got = 0;
    Tile<CC> & tile = GetTile();
    Dir dir = m_lockRegion;
    if (Dirs::IsCorner(m_lockRegion))
    {
      needed = 3;
      dir = Dirs::CCWDir(dir);
    }

    LOG.Debug("EW::AcquireRegionLocks - checking %d", needed);
    for (; got < needed; ++got, dir = Dirs::CWDir(dir))
    {
      CacheProcessor<CC> & cp = tile.GetCacheProcessor(dir);

      if (!cp.IsConnected())
      {
        // Whups, didn't really need that one.  Leave it null, since
        // the other loops check all MAX_CACHES_TO_UPDATE slots anyway
        LOG.Debug("EW::AcquireRegionLocks - skip: dir %d unconnected", dir);
        continue;
      }

      if (!cp.IsIdle())
      {
        LOG.Debug("EW::AcquireRegionLocks - fail: dir %d cp not idle", dir);
        break;  // Already otherwise engaged
      }

      bool locked = cp.TryLock();
      if (!locked)
      {
        LOG.Debug("EW::AcquireRegionLocks - fail: didn't get dir %d lock", dir);
        break; // Didn't get the lock
      }
      LOG.Debug("EW::AcquireRegionLocks #%d, dir %d, locked", got, dir);
      m_cacheProcessorsLocked[got] = &cp;
    }

    if (got < needed)
    {
      LOG.Debug("EW::AcquireRegionLocks - got %d but needed %d", got, needed);
      // Opps, didn't get all, free any we got
      for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
      {
        if (m_cacheProcessorsLocked[i])
        {
          CacheProcessor<CC> & cp = *m_cacheProcessorsLocked[i];
          cp.Unlock();
          LOG.Debug("EW::AcquireRegionLocks #%d freed", i);
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
        LOG.Debug("EW::AcquireRegionLocks activate #%d", i);
        m_cacheProcessorsLocked[i]->Activate();
      }
    }

    return true;
  }

  template <class CC>
  bool EventWindow<CC>::AcquireAllLocks(const SPoint& tileCenter)
  {
    Tile<CC> & t = GetTile();
    m_lockRegion = t.GetLockDirection(tileCenter);
    return AcquireRegionLocks();
  }

  template <class CC>
  EventWindow<CC>::EventWindow(Tile<CC> & tile)
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

  template <class CC>
  SPoint EventWindow<CC>::MapToTileValid(const SPoint& offset) const
  {
    if (!InWindow(offset)) FAIL(ILLEGAL_ARGUMENT);
    return MapToTile(offset);
  }

  template <class CC>
  u32 EventWindow<CC>::MapToIndexValid(const SPoint & loc) const
  {
    const MDist<R> & md = MDist<R>::get();
    s32 index = md.FromPoint(Map(loc,m_sym,loc),R);
    if (index < 0)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return (u32) index;
  }

  template <class CC>
  void EventWindow<CC>::LoadFromTile()
  {
    const MDist<R> & md = MDist<R>::get();
    Tile<CC> & tile = GetTile();
    for (u32 i = 0; i < SITE_COUNT; ++i)
    {
      const SPoint & pt = md.GetPoint(i) + m_center;
      m_atomBuffer[i] = *tile.GetAtom(pt);
      m_isLiveSite[i] = tile.IsLiveSite(pt);
    }
  }

  template <class CC>
  void EventWindow<CC>::StoreToTile()
  {
    LOG.Debug("EW::StoreToTile");

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
    Tile<CC> & tile = GetTile();

    for (u32 i = 0; i < SITE_COUNT; ++i)
    {
      const SPoint & pt = md.GetPoint(i) + m_center;
      bool dirty = false;
      if (m_isLiveSite[i] && m_atomBuffer[i] != *tile.GetAtom(pt))
      {
        tile.PlaceAtom(m_atomBuffer[i], pt);
        dirty = true;
      }

      // Let the CPs see even unchanged atoms, for spot checks
      for (u32 j = 0; j < maxCacheUsed; ++j)
      {
        if (m_cacheProcessorsLocked[j] != 0)
        {
          m_cacheProcessorsLocked[j]->MaybeSendAtom(m_atomBuffer[i], dirty, i);
        }
      }
    }

    LOG.Debug("EW::StoreToTile releasing up to %d", maxCacheUsed);
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

  template <class CC>
  bool EventWindow<CC>::SetRelativeAtom(const SPoint& offset, const T & atom)
  {
    u32 idx = MapToIndexValid(offset);
    if (m_isLiveSite[idx])
    {
      m_atomBuffer[idx] = atom;
      return true;
    }
    return false;
  }

  template <class CC>
  const typename CC::ATOM_TYPE& EventWindow<CC>::GetRelativeAtom(const SPoint& offset) const
  {
    return m_atomBuffer[MapToIndexValid(offset)];
  }

  template <class CC>
  const typename CC::ATOM_TYPE& EventWindow<CC>::GetRelativeAtom(const Dir mooreOffset) const
  {
    SPoint pt;
    Dirs::FillDir(pt, mooreOffset);
    return GetRelativeAtom(pt);
  }

  template <class CC>
  void EventWindow<CC>::SwapAtoms(const SPoint& locA, const SPoint& locB)
  {
    u32 idxa = MapToIndexValid(locA);
    u32 idxb = MapToIndexValid(locB);

    T tmp = m_atomBuffer[idxa];
    m_atomBuffer[idxa] = m_atomBuffer[idxb];
    m_atomBuffer[idxb] = tmp;
  }

} /* namespace MFM */
