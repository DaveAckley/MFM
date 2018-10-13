/* -*- C++ -*- */

#include "PacketIO.h"
#include "CharBufferByteSource.h"
#include "EventHistoryBuffer.h"

namespace MFM
{
  template <class EC>
  void CacheProcessor<EC>::ReportCacheProcessorStatus(Logger::Level level)
  {
    if (!m_tile)
    {
      LOG.Log(level,"   ==CP Global NO TILE (%d, %d)==",
              m_farSideOrigin.GetX(),
              m_farSideOrigin.GetY());
      return;
    }
    LOG.Log(level,"   ==CP Global %s (%d, %d)==",
            m_tile->GetLabel(),
            m_farSideOrigin.GetX(),
            m_farSideOrigin.GetY());
    LOG.Log(level,"    Address: %p", (void*) this);
    LOG.Log(level,"    State: %s", GetStateName(m_cpState));
    LOG.Log(level,"    EventCenter: (%d,%d)", m_eventCenter.GetX(), m_eventCenter.GetY());
    LOG.Log(level,"    CheckOdds: %d", m_checkOdds);
    LOG.Log(level,"    ToSendCount: %d", m_toSendCount);
    LOG.Log(level,"    SentCount:   %d", m_sentCount);

    m_channelEnd.ReportChannelEndStatus(level);
  }

  template <class EC>
  bool CacheProcessor<EC>::IsSiteNumberVisible(u16 siteNumber)
  {
    const MDist<R> & md = MDist<R>::get();

    // Get site address in full untransformed tile coordinates
    SPoint local = md.GetPoint(siteNumber) + m_eventCenter;
    SPoint mdtmp = md.GetPoint(siteNumber);
    MFM_LOG_DBG7(("site# %d, point(%d, %d), local(%d, %d)", siteNumber, mdtmp.GetX(), mdtmp.GetY(), local.GetX(), local.GetY()));
    return IsCoordVisibleToPeer(local);
  }

  template <class EC>
  bool CacheProcessor<EC>::IsCoordVisibleToPeer(const SPoint & local)
  {
    Tile<EC> & tile = GetTile();

    // Map full untransformed local tile coord into the remote space
    SPoint remote = LocalToRemote(local);

    MFM_LOG_DBG7(("peer remote(%d, %d), local(%d, %d), -farside(%d,%d)", remote.GetX(), remote.GetY(), local.GetX(), local.GetY(),m_farSideOrigin.GetX(),m_farSideOrigin.GetY()));
    return tile.IsInTile(remote);
  }

  template <class EC>
  void CacheProcessor<EC>::StartShipping()
  {
    MFM_API_ASSERT_STATE(m_cpState == LOADING);

    // Now it's about shipping
    SetStateInternal(SHIPPING);

    PacketIO pbuffer;
    if (!pbuffer.SendUpdateBegin(*this, m_eventCenter))
    {
      FAIL(ILLEGAL_STATE);
    }
  }

  template <class EC>
  void CacheProcessor<EC>::StartLoading(const SPoint & eventCenter)
  {
    MFM_API_ASSERT_STATE(m_cpState == ACTIVE);
    SetStateInternal(LOADING);
    m_eventCenter = eventCenter;
    m_toSendCount = 0;
    m_sentCount = 0;
  }

  template <class EC>
  void CacheProcessor<EC>::MaybeSendAtom(const T & atom, bool changed, u16 siteNumber)
  {
    MFM_API_ASSERT_STATE(m_cpState == LOADING);

    // If far side can't see it, done
    if (!IsSiteNumberVisible(siteNumber))
    {
      return;
    }

    Tile<EC> & t = GetTile();
    Random & random = t.GetRandom();
    const u32 checkOdds = GetCheckOdds();

    // If unchanged and not time for a redundant check, done
    if (!changed && !random.OneIn(checkOdds))
    {
      return;
    }

    // Time to pack this puppy up for travel
    MFM_API_ASSERT_STATE(m_toSendCount < SITE_COUNT);  // You say ship a whole window or more?

    MFM_LOG_DBG7(("CP %s %s %d[%s %s %s] (%d,%d) send #%d (%d,%d)",
                  m_tile->GetLabel(),
                  Dirs::GetName(m_cacheDir),
		  m_locksNeeded,
		  Dirs::GetName(m_lockRegions[0]),
		  m_locksNeeded > 1? Dirs::GetName(m_lockRegions[1]) : "-",
		  m_locksNeeded > 2? Dirs::GetName(m_lockRegions[2]) : "-",
                  m_farSideOrigin.GetX(),
                  m_farSideOrigin.GetY(),
                  siteNumber,
                  99,
                  99));

    // Allocate next struct
    CachePacketInfo & cpi = m_toSend[m_toSendCount++];

    cpi.m_atom = atom;
    cpi.m_siteNumber = siteNumber;
    cpi.m_type = changed ? PacketType::UPDATE : PacketType::CHECK;
  }

  template <class EC>
  bool CacheProcessor<EC>::ShipBufferAsPacket(PacketBuffer & pb)
  {
    MFM_API_ASSERT(!pb.HasOverflowed(), OUT_OF_ROOM);

    u32 plen = pb.GetLength();
    if (m_channelEnd.CanWrite() <= plen) // Total write will be plen+1
    {
      return false;
    }

    u8 byte = (u8) plen;  // plen<128 since OString128..
    m_channelEnd.Write(&byte, 1);  // Packet length, then data
    m_channelEnd.Write((const u8 *) pb.GetBuffer(), plen);
    return true;
  }

  template <class EC>
  void CacheProcessor<EC>::BeginUpdate(SPoint onCenter)
  {
    MFM_API_ASSERT_STATE(m_cpState == IDLE);

    SetStateInternal(PASSIVE);
    m_eventCenter = onCenter;
    m_receivedSiteCount = 0;      // Nothing stashed so far
    m_consistentAtomCount = 0;
  }

  inline static u8 csgn(s32 n)
  {
    if (n < 0) return '-';
    if (n > 0) return '+';
    return '0';
  }

  template <class EC>
  void CacheProcessor<EC>::ReceiveAtom(bool isDifferent, s32 siteNumber, const T & inboundAtom)
  {
    MFM_API_ASSERT_STATE(m_cpState == PASSIVE && m_tile != 0);
    MFM_API_ASSERT_ARG(siteNumber >= 0 && siteNumber < SITE_COUNT);
    if(!IsSiteNumberVisible((u16) siteNumber))
      ReportCacheProcessorStatus((Logger::Level)1);  //debug for elena
    MFM_API_ASSERT_ARG(IsSiteNumberVisible((u16) siteNumber));
    MFM_API_ASSERT_STATE(m_receivedSiteCount < SITE_COUNT);

    m_receivedSiteNumbers[m_receivedSiteCount] = siteNumber;
    m_receivedSiteBuffer[m_receivedSiteCount] = inboundAtom;
    m_receivedSiteDifferents[m_receivedSiteCount] = isDifferent;
    ++m_receivedSiteCount;
  }

  template <class EC>
  void CacheProcessor<EC>::ApplyCacheUpdate()
  {
    EventHistoryBuffer<EC> & ehb = m_tile->GetEventHistoryBuffer();
    ehb.AddEventStart(m_eventCenter);

    const MDist<R> & md = MDist<R>::get();
    for (u32 i = 0; i < m_receivedSiteCount; ++i)
    {
      u32 siteNumber = m_receivedSiteNumbers[i];
      T& inboundAtom = m_receivedSiteBuffer[i];
      bool isDifferent = m_receivedSiteDifferents[i];

      const SPoint soffset = md.GetPoint(siteNumber);
      SPoint loc = soffset + m_eventCenter;
      const T& oldAtom = *m_tile->GetAtom(loc);

      if (isDifferent)
      {
        ehb.AddEventAtom(siteNumber, oldAtom, inboundAtom);
      }

      bool consistent =
        m_tile->ApplyCacheUpdate(isDifferent, inboundAtom, loc);

      if (consistent)
      {
        ++m_consistentAtomCount;
      }
      else
      {
        if (LOG.IfLog(Logger::WARNING))
        {
          // Get old for debug output
          T oldAtomx = oldAtom; //*m_tile->GetAtom(loc);

          AtomSerializer<AC> oldas(oldAtomx);
          AtomSerializer<AC> as(inboundAtom);

          // Develop a secret grid-global address! shh don't tell ackley!
          s32 tilex = -1, tiley = -1;
          const char * tlb = GetTile().GetLabel();
          CharBufferByteSource cbs(tlb,strlen(tlb));
          cbs.Scanf("[%d,%d]",&tilex,&tiley);

	  const SPoint ownedp(GetTile().OWNED_WIDTH, GetTile().OWNED_HEIGHT);
	  SPoint gloc = SPoint(tilex,tiley) * ownedp + loc;

          LOG.Warning("NC%s %s%c%c {%03d,%03d} #%2d(%2d,%2d)+(%2d,%2d)==(%2d,%2d) [%04x/%@] [%04x/%@]",
                      isDifferent? "U" : "C",
                      tlb,
                      csgn(m_farSideOrigin.GetX()),
                      csgn(m_farSideOrigin.GetY()),
                      gloc.GetX(),
                      gloc.GetY(),
                      siteNumber,
                      soffset.GetX(),
                      soffset.GetY(),
                      m_eventCenter.GetX(),
                      m_eventCenter.GetY(),
                      loc.GetX(),
                      loc.GetY(),
                      inboundAtom.GetType(),
                      &as,
                      oldAtom.GetType(),
                      &oldas);
        }
      }
    }
    ehb.AddEventEnd();
  }

  template <class EC>
  void CacheProcessor<EC>::ReceiveUpdateEnd()
  {
    MFM_API_ASSERT_STATE(m_cpState == PASSIVE);
    MFM_LOG_DBG7(("Replying to UE, %d consistent",m_consistentAtomCount));
    ApplyCacheUpdate();
    PacketIO pbuffer;
    pbuffer.SendReply(m_consistentAtomCount, *this);
    SetIdle();
  }

  template <class EC>
  void CacheProcessor<EC>::ReceiveReply(u32 consistentCount)
  {
    MFM_API_ASSERT_STATE(m_cpState == RECEIVING);

    if (consistentCount != m_toSendCount)
    {
      ReportCheckFailure();
    }
    else
    {
      ReportCleanUpdate(m_toSendCount);
    }
    MFM_LOG_DBG7(("CP %s %s %d[%s %s %s] reply %d<->%d : %d",
                  GetTile().GetLabel(),
                  Dirs::GetName(m_cacheDir),
		  m_locksNeeded,
		  Dirs::GetName(m_lockRegions[0]),
		  m_locksNeeded > 1? Dirs::GetName(m_lockRegions[1]) : "-",
		  m_locksNeeded > 2? Dirs::GetName(m_lockRegions[2]) : "-",
                  consistentCount,
                  m_toSendCount,
                  m_checkOdds));

    SetStateInternal(BLOCKING);
  }

  template <class EC>
  bool CacheProcessor<EC>::Advance()
  {
    // If we're unconnected, we're not working
    if (!m_tile)
    {
      return false;
    }

    if (m_cpState != IDLE)
    {
      MFM_LOG_DBG7(("CP %s %s %d[%s %s %s] Advance in state %s",
                    GetTile().GetLabel(),
                    Dirs::GetName(m_cacheDir),
		    m_locksNeeded,
		    Dirs::GetName(m_lockRegions[0]),
		    m_locksNeeded > 1? Dirs::GetName(m_lockRegions[1]) : "-",
		    m_locksNeeded > 2? Dirs::GetName(m_lockRegions[2]) : "-",
                    GetStateName(m_cpState)));
    }

    switch (m_cpState)
    {
    case SHIPPING: return AdvanceShipping();
    case IDLE:    // During idle we just receive
    case PASSIVE: // During passive we receive
    case RECEIVING: return AdvanceReceiving();
    case BLOCKING: return AdvanceBlocking();
    default:
      FAIL(ILLEGAL_STATE);
    }
  }

  template <class EC>
  void CacheProcessor<EC>::Activate()
  {
    MFM_API_ASSERT_STATE(m_cpState == IDLE);
    SetStateInternal(ACTIVE);
  }

  template <class EC>
  void CacheProcessor<EC>::SetIdle()
  {
    MFM_API_ASSERT_STATE(m_cpState != IDLE);
    SetStateInternal(IDLE);
  }

  template <class EC>
  bool CacheProcessor<EC>::AdvanceShipping()
  {
    MFM_LOG_DBG7(("CP %s %s %d[%s %s %s] (%d,%d): Advance shipping",
                  GetTile().GetLabel(),
                  Dirs::GetName(m_cacheDir),
		  m_locksNeeded,
		  Dirs::GetName(m_lockRegions[0]),
		  m_locksNeeded > 1? Dirs::GetName(m_lockRegions[1]) : "-",
		  m_locksNeeded > 2? Dirs::GetName(m_lockRegions[2]) : "-",
                  m_farSideOrigin.GetX(),
                  m_farSideOrigin.GetY()));
    bool didWork = false;
    PacketIO pbuffer;

    // Try to send any unsent packets
    while (m_sentCount < m_toSendCount)
    {
      CachePacketInfo & cpi = m_toSend[m_sentCount];
      if (!pbuffer.SendAtom(cpi.m_type, *this, cpi.m_siteNumber, cpi.m_atom))
      {
        return didWork;
      }
      didWork = true;
      ++m_sentCount;
      MFM_LOG_DBG7(("CP %s %s %d[%s %s %s]: Ship %d (site #%d)",
                    GetTile().GetLabel(),
                    Dirs::GetName(m_cacheDir),
		    m_locksNeeded,
		    Dirs::GetName(m_lockRegions[0]),
		    m_locksNeeded > 1? Dirs::GetName(m_lockRegions[1]) : "-",
		    m_locksNeeded > 2? Dirs::GetName(m_lockRegions[2]) : "-",
                    m_sentCount,
                    cpi.m_siteNumber));
    }

    // Try to send the update end packet if not yet sent
    if (m_sentCount == m_toSendCount)
    {
      if (!pbuffer.SendUpdateEnd(*this))
      {
        return didWork;
      }
      didWork = true;
    }

    SetStateInternal(RECEIVING);
    return didWork;
  }

  template <class EC>
  bool CacheProcessor<EC>::AdvanceReceiving()
  {
    bool didWork = false;
    if (!IsConnected())
    {
      return didWork;
    }

    PacketIO pio;
    while (true)
    {
      PacketBuffer * pb = m_channelEnd.ReceivePacket();
      if (!pb || pb->GetLength() == 0)
      {
        return didWork;
      }
      didWork = true;
      if (!pio.HandlePacket(*this, *pb))
      {
        FAIL(INCOMPLETE_CODE);
      }
    }
  }

  template <class EC>
  void CacheProcessor<EC>::Unblock()
  {
    MFM_API_ASSERT_STATE(m_cpState == BLOCKING);
    MFM_LOG_DBG7(("CP::Unblock %s %s %d[%s %s %s] (%d,%d)",
                  m_tile->GetLabel(),
                  Dirs::GetName(m_cacheDir),
		  m_locksNeeded,
		  Dirs::GetName(m_lockRegions[0]),
		  m_locksNeeded > 1? Dirs::GetName(m_lockRegions[1]) : "-",
		  m_locksNeeded > 2? Dirs::GetName(m_lockRegions[2]) : "-",
                  m_farSideOrigin.GetX(),
                  m_farSideOrigin.GetY())
		 );
    SetIdle();
    Unlock();  // FINALLY
  }

  template <class EC>
  CacheProcessor<EC> & CacheProcessor<EC>::GetSibling(Dir forDirection)
  {
    MFM_API_ASSERT_STATE(m_tile);

    return m_tile->GetCacheProcessor(forDirection);
  }


  template <class EC>
  bool CacheProcessor<EC>::AdvanceBlocking()
  {
    u32 needed = m_locksNeeded;

    THREEDIR copylockdirs; //copy before m_lockRegions cleared by Unblock
    for (u32 d = 0; d < MAX_LOCKS_NEEDED; d++)  copylockdirs[d] = m_lockRegions[d];

    // Check if every-relevant-body is blocking
    u32 got = 0;
    for (u32 d = 0; got < needed; ++got, d++)
    {
      CacheProcessor<EC> & cp = GetSibling(copylockdirs[d]);
      if(cp.IsConnected() && !cp.IsBlocking())
	{
	  break;
	}
      //else incr got
    }

    if (got < needed)
    {
      return false;  // Somebody else is still working
    }

    // All are done, unblock all, including ourselves
    got = 0;
    for (u32 d = 0; got < needed; ++got, d++)
      {
	CacheProcessor<EC> & cp = GetSibling(copylockdirs[d]);
	if (cp.IsConnected())
	  {
	    cp.Unblock();
	  }
      }

    return true;
  }

}
