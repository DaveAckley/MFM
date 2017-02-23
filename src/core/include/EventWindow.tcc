/* -*- C++ -*- */
#include "MDist.h"
#include "Element.h"
#include "Tile.h"
#include "ChannelEnd.h"
#include "PacketIO.h"
#include "EventHistoryBuffer.h"
#include "CacheProcessor.h"

namespace MFM {

  template <class EC>
  bool EventWindow<EC>::TryForceEventAt(const SPoint & tcenter)
  {
    MFM_LOG_DBG6(("EW::TryForceEventAt(%d,%d)",
                  tcenter.GetX(),
                  tcenter.GetY()));
    ++m_eventWindowsAttempted;

    if (!InitForEvent(tcenter))
    {
      return false;
    }

    RecordEventAtTileCoord(tcenter);
    ExecuteEvent();

    return true;
  }


  template <class EC>
  bool EventWindow<EC>::TryEventAt(const SPoint & tcenter)
  {
    MFM_LOG_DBG6(("EW::TryEventAt(%d,%d)",
                  tcenter.GetX(),
                  tcenter.GetY()));
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
    //t.GetSite(owned).SetLastEventEventNumber(m_eventWindowsExecuted);
    t.GetSite(tcoord).RecordEventAtSite(m_eventWindowsExecuted);
  }

  template <class EC>
  bool EventWindow<EC>::RejectOnRecency(const SPoint tcoord)
  {
    Tile<EC> & t = GetTile();
    const u32 warpFactor = t.GetWarpFactor();
    SPoint owned = Tile<EC>::TileCoordToOwned(tcoord);
    u32 eventAge = t.GetUncachedEventAge32(owned);  // max age at one billion
    return !GetRandom().OddsOf(eventAge + warpFactor*t.GetSites(), 10*t.GetSites());
  }

  template <class EC>
  void EventWindow<EC>::ExecuteEvent()
  {
    MFM_LOG_DBG6(("EW::ExecuteEvent"));
    MFM_API_ASSERT_STATE(m_ewState == COMPUTE);

    ExecuteBehavior();

    InitiateCommunications();
  }

  template <class EC>
  void EventWindow<EC>::InitiateCommunications()
  {
    MFM_LOG_DBG6(("EW::InitiateCommunications"));
    MFM_API_ASSERT_STATE(m_ewState == COMPUTE);

    // Step 1: Write back the event window and set up the CacheProcessors
    StoreToTile();

    // Step 2: We are done!
    m_ewState = FREE;
  }

  template <class EC>
  void EventWindow<EC>::PrintEventSite(ByteSink & bs)
  {
    Tile<EC> & t = GetTile();
    SPointSerializer ssp(m_center);
    bs.Printf("T%s@S%@", t.GetLabel(), &ssp);
  }

  template <class EC>
  void EventWindow<EC>::ExecuteBehavior()
  {
    MFM_LOG_DBG6(("EW::ExecuteBehavior"));
    Tile<EC> & t = GetTile();
    unwind_protect(
    {
      OString256 buff;
      PrintEventSite(buff);
      buff.Printf(":");

      const char * failFile = MFMThrownFromFile;
      const unsigned lineno = MFMThrownFromLineNo;
      const char * failMsg = MFMFailCodeReason(MFMThrownFailCode);
      if(!GetCenterAtomDirect().IsSane())
      {
        LOG.Debug("%s FE(INSANE)",buff.GetZString());
      }
      else if (failMsg)
      {
        LOG.Message("%s behave() failed at %s:%d: %s (site type 0x%04x)",
                    buff.GetZString(),
                    failFile,
                    lineno,
                    failMsg,
                    GetCenterAtomDirect().GetType());
      }
      else
      {
        LOG.Message("%s behave() failed at %s:%d: fail(%d/0x%08x) (site type 0x%04x)",
                    buff.GetZString(),
                    failFile,
                    lineno,
                    MFMThrownFailCode,
                    MFMThrownFailCode,
                    GetCenterAtomDirect().GetType());
      }

      SetCenterAtomDirect(t.GetEmptyAtom());
    },
    {
      MFM_LOG_DBG6(("ET::Execute"));
      m_element->Behavior(*this);
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
  void EventWindow<EC>::SetBoundary(u32 boundary)
  {
    if (boundary > R + 1) boundary = R + 1;

    m_eventWindowBoundary = boundary;
    const MDist<R> & md = MDist<R>::get();
    m_boundedSiteCount = md.GetFirstIndex(m_eventWindowBoundary);
  }

  template <class EC>
  bool EventWindow<EC>::InitForEvent(const SPoint & center)
  {
    MFM_LOG_DBG6(("EW::InitForEvent(%d,%d)",center.GetX(),center.GetY()));

    MFM_API_ASSERT_STATE(IsFree());  // Don't be callin' when I'm not free

    Tile<EC> & tile = GetTile();

    // We need to access the element early to determine its boundary
    T atom = *tile.GetAtom(center);
    if (!atom.IsSane())
    {
      OString256 buff;
      PrintEventSite(buff);
      buff.Printf(": INSANE ATOM ");
      bool fixed = atom.HasBeenRepaired();

      if (fixed)
      {
        buff.Printf("REPAIRED");
        tile.PlaceAtom(atom, center);
      }
      else
      {
        buff.Printf("ERASED");
        tile.PlaceAtom(tile.GetEmptyAtom(), center);
      }

      LOG.Debug("%s",buff.GetZString());
      if (!fixed)
        return false;
    }

    u32 type = atom.GetType();
    m_element  = tile.GetElementTable().Lookup(type);
    if (m_element == 0) // If no element of that type
    {
      tile.PlaceAtom(tile.GetEmptyAtom(), center);  // You must die
      return false;
    }

    SetBoundary(m_element->GetEventWindowBoundary());

    if (!AcquireAllLocks(center, m_eventWindowBoundary))
    {
      MFM_LOG_DBG6(("EW::InitForEvent - abandoned"));
      return false;
    }

    m_eventWindowSitesAccessed += m_boundedSiteCount;
    m_center = center;
    m_ewState = COMPUTE;
    m_sym = PSYM_NORMAL;

    LoadFromTile();
    return true;
  }

  template <class EC>
  typename EventWindow<EC>::LockStatus EventWindow<EC>::AcquireDirLock(Dir dir)
  {
    CacheProcessor<EC> & cp = GetTile().GetCacheProcessor(dir);

    if (!cp.IsConnected())
    {
      // Whups, didn't really need that one.  Leave it null, since
      // the other loops check all MAX_CACHES_TO_UPDATE slots anyway
      MFM_LOG_DBG6(("EW::AcquireRegionLocks - skip: %s unconnected",
                    Dirs::GetName(dir)));
      return LOCK_UNNEEDED;
    }

    if (!cp.IsIdle())
    {
      MFM_LOG_DBG6(("EW::AcquireRegionLocks - fail: %s cp not idle",
                    Dirs::GetName(dir)));
      return LOCK_UNAVAILABLE;
    }

    bool locked = cp.TryLock(m_lockRegion);
    if (!locked)
    {
      MFM_LOG_DBG6(("EW::AcquireRegionLocks - fail: didn't get %s lock",
                    Dirs::GetName(dir)));
      return LOCK_UNAVAILABLE;
    }
    MFM_LOG_DBG6(("EW::AcquireRegionLocks, %s locked",
                  Dirs::GetName(dir)));
    return LOCK_ACQUIRED;
  }

  template <class EC>
  bool EventWindow<EC>::AcquireRegionLocks()
  {
    Random & random = GetRandom();

    MFM_LOG_DBG6(("EW::AcquireRegionLocks"));
    // We cannot still have any cacheprocessors in use
    for (u32 i = 0; i < MAX_CACHES_TO_UPDATE; ++i)
    {
      MFM_API_ASSERT_STATE(m_cacheProcessorsLocked[i] == 0);
    }

    if (((s32) m_lockRegion) == -1)
    {
      MFM_LOG_DBG6(("EW::AcquireRegionLocks - none needed"));
      return true;  // Nobody is needed
    }

    // At least one lock may be needed

    Dir lockDirs[3];
    u32 needed = 1;
    Tile<EC> & tile = GetTile();
    lockDirs[0] = m_lockRegion;

    if (Dirs::IsCorner(m_lockRegion))
    {
      lockDirs[1] = Dirs::CCWDir(m_lockRegion);
      lockDirs[2] = Dirs::CWDir(m_lockRegion);
      needed = 3;
      Shuffle<Dir,3>(GetRandom(),lockDirs);
    }

    MFM_LOG_DBG6(("EW::AcquireRegionLocks - checking %d", needed));

    u32 got = 0;
    for (s32 i = needed; --i >= 0; )
    {
      Dir dir = lockDirs[i];
      LockStatus ls = AcquireDirLock(dir);

      if (ls == LOCK_UNNEEDED)
      {
        // Whups, didn't really need that one.  Leave it null, since
        // the other loops check all MAX_CACHES_TO_UPDATE slots anyway
        --needed;
        continue;
      }

      if (ls == LOCK_UNAVAILABLE)
      {
        break;  // Not idle or we didn't get it
      }

      /* ls == LOCK_ACQUIRED */

      CacheProcessor<EC> & cp = tile.GetCacheProcessor(dir);
      m_cacheProcessorsLocked[got] = &cp;
      ++got;
    }

    if (got < needed)
    {
      MFM_LOG_DBG6(("EW::AcquireRegionLocks - got %d but needed %d", got, needed));
      // Opps, didn't get all, free any we got

      for (m_cpli.ShuffleOrReset(random); m_cpli.HasNext(); )
      {
        u32 i = m_cpli.Next();

        if (m_cacheProcessorsLocked[i])
        {
          CacheProcessor<EC> & cp = *m_cacheProcessorsLocked[i];
          cp.Unlock();
          MFM_LOG_DBG6(("EW::AcquireRegionLocks #%d freed", i));
          m_cacheProcessorsLocked[i] = 0;
        }
      }

      return false;
    }

    // Activate everything we got
    for (m_cpli.ShuffleOrReset(random); m_cpli.HasNext(); )
    {
      u32 i = m_cpli.Next();

      if (m_cacheProcessorsLocked[i])
      {
        MFM_LOG_DBG6(("EW::AcquireRegionLocks activate #%d", i));
        m_cacheProcessorsLocked[i]->Activate();
      }
    }

    return true;
  }

  template <class EC>
  bool EventWindow<EC>::AcquireAllLocks(const SPoint& tileCenter, const u32 eventWindowBoundary)
  {
    Tile<EC> & t = GetTile();
    m_lockRegion = t.GetLockDirection(tileCenter, eventWindowBoundary);
    return AcquireRegionLocks();
  }

  template <class EC>
  EventWindow<EC>::EventWindow(Tile<EC> & tile)
    : m_tile(tile)
    , m_eventWindowBoundary(R + 1)
    , m_boundedSiteCount(SITE_COUNT)
    , m_element(0)
    , m_eventWindowsAttempted(0)
    , m_eventWindowsExecuted(0)
    , m_eventWindowSitesAccessed(0)
    , m_center(0,0)
    , m_lockRegion(-1)
    , m_sym(PSYM_NORMAL)
    , m_ewState(FREE)
  {
    m_cpli.Shuffle(GetRandom());

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
    MFM_API_ASSERT_ARG(InWindow(offset));
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
    MFM_API_ASSERT_ARG(index >= 0 && ((u32) index) < m_boundedSiteCount);
    return (u32) index;
  }

  template <class EC>
  void EventWindow<EC>::LoadFromTile()
  {
    Tile<EC> & tile = GetTile();

    m_centerBase = tile.GetSite(m_center).GetBase();
    const MDist<R> & md = MDist<R>::get();
    for (u32 i = 0; i < m_boundedSiteCount; ++i)
    {
      const SPoint & pt = md.GetPoint(i) + m_center;
      //m_atomBuffer[i] = tile.GetAtomForEventWindow(pt);
      m_atomBuffer[i].WriteAtom(tile.GetAtomForEventWindow(pt));
      m_isLiveSite[i] = tile.IsLiveSite(pt);
    }
  }

  template <class EC>
  void EventWindow<EC>::StoreToTile()
  {
    const MDist<R> & md = MDist<R>::get();

    Random & random = GetRandom();

    MFM_LOG_DBG6(("EW::StoreToTile"));

    // First initialize the cache processors
    for (m_cpli.ShuffleOrReset(random); m_cpli.HasNext(); )
    {
      u32 i = m_cpli.Next();

      if (m_cacheProcessorsLocked[i])
      {
        m_cacheProcessorsLocked[i]->StartLoading(m_center);
      }
    }

    // Now write back changes and notify the cps
    Tile<EC> & tile = GetTile();

    // Record the event in the tile history
    EventHistoryBuffer<EC> & ehb = tile.GetEventHistoryBuffer();
    ehb.AddEventWindow(*this);

    // Write back base changes if any
    tile.GetSite(m_center).GetBase() = m_centerBase;

    for (u32 i = 0; i < m_boundedSiteCount; ++i)
    {
      const SPoint & pt = md.GetPoint(i) + m_center;
      bool dirty = false;
      if (m_isLiveSite[i])
      {
        //if (m_atomBuffer[i] != *tile.GetAtom(pt))
	if (m_atomBuffer[i].GetAtom() != *tile.GetAtom(pt))
        {
          tile.PlaceAtom(m_atomBuffer[i].GetAtom(), pt);
          dirty = true;
        }

        // Let the CPs see even some unchanged atoms, for spot checks
        for (u32 j = 0; j < MAX_CACHES_TO_UPDATE; ++j)
        {
          if (m_cacheProcessorsLocked[j] != 0)
          {
            m_cacheProcessorsLocked[j]->MaybeSendAtom( *tile.GetAtom(pt), dirty, i);
          }
        }
      }
    }

    MFM_LOG_DBG6(("EW::StoreToTile releasing"));
    // Finally, release the cache processors to take it from here
    for (m_cpli.ShuffleOrReset(random); m_cpli.HasNext(); )
    {
      u32 i = m_cpli.Next();
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
    MFM_API_ASSERT_ARG(idx < m_boundedSiteCount);

    if (m_isLiveSite[idx])
    {
      //m_atomBuffer[idx] = atom;
      m_atomBuffer[idx].WriteAtom(atom); //a copy
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
      //m_atomBuffer[idx] = atom;
      m_atomBuffer[idx].WriteAtom(atom);
      return true;
    }
    return false;
  }

  template <class EC>
  const typename EC::ATOM_CONFIG::ATOM_TYPE& EventWindow<EC>::GetRelativeAtomSym(const SPoint& offset) const
  {
    u32 idx = MapToIndexSymValid(offset);
    MFM_API_ASSERT_ARG(idx < m_boundedSiteCount);
    //return m_atomBuffer[idx];
    return m_atomBuffer[idx].GetAtom();
  }

  template <class EC>
  const typename EC::ATOM_CONFIG::ATOM_TYPE& EventWindow<EC>::GetRelativeAtomDirect(const SPoint& offset) const
  {
    u32 idx = MapToIndexDirectValid(offset);
    //return m_atomBuffer[idx];
    return m_atomBuffer[idx].GetAtom();
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
    SwapAtomsDirect(idxa, idxb);
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsDirect(const u32 idxa, const u32 idxb)
  {
    MFM_API_ASSERT_ARG(idxa < m_boundedSiteCount);
    MFM_API_ASSERT_ARG(idxb < m_boundedSiteCount);

    T tmp = m_atomBuffer[idxa].GetAtom();
    //m_atomBuffer[idxa] = m_atomBuffer[idxb];
    //m_atomBuffer[idxb] = tmp;
    m_atomBuffer[idxa].WriteAtom(m_atomBuffer[idxb].GetAtom());
    m_atomBuffer[idxb].WriteAtom(tmp);
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsSym(const SPoint& locA, const SPoint& locB)
  {
    u32 idxa = MapToIndexSymValid(locA);
    u32 idxb = MapToIndexSymValid(locB);
    SwapAtomsDirect(idxa, idxb);
  }

  template <class EC>
  void EventWindow<EC>::SwapAtomsDirect(const SPoint& locA, const SPoint& locB)
  {
    u32 idxa = MapToIndexDirectValid(locA);
    u32 idxb = MapToIndexDirectValid(locB);
    SwapAtomsDirect(idxa, idxb);
  }

} /* namespace MFM */
