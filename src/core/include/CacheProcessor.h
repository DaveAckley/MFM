/*                                              -*- mode:C++ -*-
  CacheProcessor.h Handler for cache-protocol packets
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
  \file CacheProcessor.h Handler for cache-protocol packets
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CACHEPROCESSOR_H
#define CACHEPROCESSOR_H

#include "itype.h"
#include "Fail.h"
#include "Point.h"
#include "Packet.h"
#include "ChannelEnd.h"
#include "MDist.h"  /* for EVENT_WINDOW_SITES */
#include "Logger.h"

namespace MFM {

  template <class CC> class Tile; // FORWARD

  /**
    CacheProcessors mediate both sides of the intertile cache update
    protocol, interfacing below with the ChannelEnd for raw packet
    transport, and above with the Tile and EventWindow, for the atoms
    and their metadata to be transported.
  */
  template <class CC>
  class CacheProcessor
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { SITE_COUNT = EVENT_WINDOW_SITES(R) };
    enum { TILE_WIDTH = P::TILE_WIDTH };

  private:

    /**
       Where to apply inbound cache updates (and where outbound cache
       updates ultimately originate, though we don't care about that.)
     */
    Tile<CC> * m_tile;

    /**
       The lock controlling who can be the active side of a
       transaction involving this cache processor.  Long-lived locks
       on edges are shared by up to two cache processors; those on
       corners, by up to four.
     */
    LonglivedLock * m_longlivedLock;

    /**
       Our cache direction, used as an owner index for the long lived
       lock, since any given lock can't be claimed from the same
       direction twice.
     */
    u32 m_cacheDir;

    /**
       The 'center region' of the event we are part of.  If
       m_centerRegion is an edge, it must be equal to m_cacheDir, and
       that implies we are the only CP involved in this event, so we
       can unlock as soon as we are done.  If m_centerRegion is a
       corner and unequal to m_cacheDir, we enter BLOCKING when we are
       done and do not unlock ourselves.  If m_centerRegion is a
       corner and equal to m_cacheDir, we enter BLOCKING when we are
       done, and then while BLOCKING, we check if both our neighbors
       are also BLOCKING.  When they are, we unlock all three of us.
     */
    Dir m_centerRegion;

    enum {
      /**
         MIN_CHECK_ODDS is the minimum value of m_checkOdds.
         1-in-MIN_CHECK_ODDS is the \e maximum amount of redundancy
         added to packet transfers to monitor remote cache quality.
         Normally has a value of 1, meaning that in the worst case, \e
         all atoms in the event window will be sent regardless of
         whether they changed or not
       */
      MIN_CHECK_ODDS = 1,

      /**
         MAX_CHECK_ODDS is the maximum value of m_checkOdds.
         1-in-MAX_CHECK_ODDS is the \e minimum amount of redundancy
         added to packet transfers to monitor remote cache quality.
         If a cache failure is detected, m_checkOdds will be reduced
         below this value, causing the \e actual redundancy to be
         higher, at least temporarily.
       */
      MAX_CHECK_ODDS = 20,

      /**
         INITIAL_CHECK_ODDS is the initial value of m_checkOdds.
         \sa MIN_CHECK_ODDS
         \sa MAX_CHECK_ODDS
         \sa m_checkOdds
       */
      INITIAL_CHECK_ODDS = 1
    };

    /**
       The current odds of including redundant check packets in the
       cache update stream.  Atoms that did \e not change during an
       event will also be transmitted, using type PACKET_CHECK, with
       odds of 1-in-this.  Minimum value is MIN_CHECK_ODDS; maximum
       value is MAX_CHECK_ODDS.
     */
    u32 m_checkOdds;

    /**
       The current number of consistent atoms acked since the last
       possibility of increasing m_checkOdds.
     */
    u32 m_remoteConsistentAtomCount;

    /**
       If true, automatically adjust m_checkOdds based on the reported
       accuracy of the remote cache content; otherwise leave
       m_checkOdds unchanged.
     */
    bool m_useAdaptiveRedundancy;

    u32 GetCheckOdds() const
    {
      return m_checkOdds;
    }

    void ReportCheckFailure()
    {
      if (m_useAdaptiveRedundancy)
      {
        m_checkOdds = MIN_CHECK_ODDS;
      }
    }

    void ReportCleanUpdate(u32 consistentAtoms)
    {
      if (m_useAdaptiveRedundancy)
      {
        m_remoteConsistentAtomCount += consistentAtoms;
        if (m_remoteConsistentAtomCount >= SITE_COUNT)
        {
          m_remoteConsistentAtomCount -= SITE_COUNT;
          if (m_checkOdds < MAX_CHECK_ODDS)
          {
            ++m_checkOdds;
          }
        }
      }
    }

    /**
       Return true if the site at the given siteNumber, relative to
       our m_eventCenter (which is measured in full untransformed Tile
       coordinates), can be seen by our peer cache processor on its
       neighboring tile.
     */
    bool IsSiteNumberVisible(u16 siteNumber) ;

    /**
       Return true if coord 'local', measured in full untransformed
       Tile coordinates, can be seen by our peer cache processor on
       its neighboring tile.
     */
    bool IsCoordVisibleToPeer(const SPoint & local) ;

    struct CachePacketInfo {
      T m_atom;                // What to send
      u16 m_siteNumber;        // Where it lives
      PacketTypeCode m_type;   // PACKET_UPDATE or PACKET_CHECK, currently
    };
    CachePacketInfo m_toSend[SITE_COUNT];
    u32 m_toSendCount;    // Used length of m_toSend
    u32 m_sentCount;      // Next index to send in m_toSend

    enum State
    {
      IDLE,         // Unlocked, not in use
      ACTIVE,       // Locked by us, event behavior is running
      LOADING,      // Locked by us, event changes loading into m_toSend
      SHIPPING,     // Locked by us, shipping event changes to peer
      RECEIVING,    // Locked by us, waiting for ack/nak from peer
      BLOCKING,     // Locked by us, waiting for other linked CP's
      PASSIVE,      // Locked by them, Received BeginUpdate from peer
      STATE_COUNT
    };

    static const char * GetStateName(const State s)
    {
      switch (s)
      {
      case IDLE: return "IDLE";
      case ACTIVE: return "ACTIVE";
      case LOADING: return "LOADING";
      case SHIPPING: return "SHIPPING";
      case RECEIVING: return "RECEIVING";
      case BLOCKING: return "BLOCKING";
      case PASSIVE: return "PASSIVE";
      default: return "illegal state";
      }
    }

    State m_cpState;

    /**
       The center position of the event we are dealing with, in full
       untransformed local Tile coordinates.
     */
    SPoint m_eventCenter;

    /**
       The count of consistent atoms received in the current update,
       when we are on the passive side -- where 'consistent' means
       that an atom sent UPDATE was indeed changed from what we had,
       and an atom sent CHECK was indeed identical to what we had.
       This value is returned to the active side in an REPLY_ACK packet
       send UPDATE_NAK.
     */
    u32 m_consistentAtomCount;

    /**
      The position of the far side's origin, in our full untransformed
      coordinate system.  This value is (+- Tile::OWNED_SIDE, +-
      Tile::OWNED_SIDE) depending on the direction to the far side.
      (It's OWNED_SIDE and not TILE_SIDE, even though we're talking
      full Tile coordinates, because of the cache overlaps.)
     */
    SPoint m_farSideOrigin;

    /**
       Where to send and receive packets.
     */
    ChannelEnd m_channelEnd;

    void SetStateInternal(State state)
    {
      MFM_LOG_DBG6(("CP %s %s [%s] (%d,%d): %s->%s",
                    m_tile->GetLabel(),
                    Dirs::GetName(m_cacheDir),
                    Dirs::GetName(m_centerRegion),
                    m_farSideOrigin.GetX(),
                    m_farSideOrigin.GetY(),
                    GetStateName(m_cpState),
                    GetStateName(state)));
      m_cpState = state;
    }

  public:
    enum RedundancyOdds {
      MAX,
      MIN,
      INITIAL,
      ADAPTIVE
    };

    u32 GetCurrentCacheRedundancy() const
    {
      return m_checkOdds;
    }

    void SetCacheRedundancy(u32 type)
    {
      m_useAdaptiveRedundancy = false;
      switch (type)
      {
      case MAX:
        m_checkOdds = MAX_CHECK_ODDS;
        break;
      case MIN:
        m_checkOdds = MIN_CHECK_ODDS;
        break;
      case INITIAL:
        m_checkOdds = INITIAL_CHECK_ODDS;
        break;
      case ADAPTIVE:
        m_useAdaptiveRedundancy = true;
      default:
        FAIL(ILLEGAL_ARGUMENT);
      }
    }

    void ReportCacheProcessorStatus(Logger::Level level) ;

    /**
       Shift this cache processor from IDLE to ACTIVE.
     */
    void Activate() ;

    /**
       Shift this cache processor back to IDLE.
     */
    void SetIdle() ;

    /**
       Initialize for an active-side cache update protocol, for an
       event window located at eventCenter.
     */
    void StartLoading(const SPoint & eventCenter) ;

    /**
       Notify the CacheProcessor that m_toSend is now fully loaded and
       no more MaybeSendAtoms will occur for this event.
     */
    void StartShipping() ;

    /**
       Advance the CacheProcessor state however it can be advanced.
       Return true if any work was done (packets sent or received,
       state changed).  Note that a false return does \e not mean that
       the CacheProcessor is necessarily IDLE; use IsIdle() to make
       that determination.
     */
    bool Advance() ;

    bool AdvanceShipping() ;

    bool AdvanceReceiving() ;

    bool AdvanceBlocking() ;

    void Unblock() ;

    bool IsIdle()
    {
      return m_cpState == IDLE;
    }

    bool IsBlocking()
    {
      return m_cpState == BLOCKING;
    }

    CacheProcessor & GetSibling(Dir inDirection) ;

    /**
       Check if this atom is visible to far end cache, and if so,
       maybe save it for shipment.
     */
    void MaybeSendAtom(const T & atom, bool changed, u16 siteNumber) ;

    /**
       Handle an inbound atom that our neighbor cache processor
       decided to MaybeSendAtom to us.
     */
    void ReceiveAtom(bool isDifferent, s32 siteNumber, const T & inboundAtom) ;

    /**
       Handle the update end that our neighbor cache processor sent us
       to end the set of atoms it sent.
     */
    void ReceiveUpdateEnd() ;

    /**
       Handle the ACK that our neighbor cache processor sent us
       in reply to our update end.
     */
    void ReceiveReply(u32 consistentCount) ;

    /**
       Record that we are (about to be) receiving an update from a window at onCenter
     */
    void BeginUpdate(SPoint onCenter) ;

    bool ShipBufferAsPacket(PacketBuffer & pb) ;

    bool TryLock(Dir centerRegion)
    {
      bool ret = GetLonglivedLock().TryLock(this);
      if (ret)
      {
        m_centerRegion = centerRegion;
      }
      return ret;
    }

    void Unlock()
    {
      bool ret = GetLonglivedLock().Unlock(this);
      if (!ret)
      {
        FAIL(LOCK_FAILURE);
      }
      m_centerRegion = (Dir) -1;
    }

    bool IsConnected() const
    {
      return m_channelEnd.IsConnected();
    }

    void ClaimCacheProcessor(Tile<CC>& tile, AbstractChannel& channel, LonglivedLock & lock, Dir toCache)
    {
      if (m_tile || m_longlivedLock)
      {
        FAIL(ILLEGAL_STATE);
      }

      m_tile = &tile;
      m_longlivedLock = &lock;
      m_cacheDir = toCache;

      // Map their full untransformed origin to our full untransformed frame
      SPoint remoteOrigin = Dirs::GetOffset(m_cacheDir) * Tile<CC>::OWNED_SIDE;
      m_farSideOrigin = remoteOrigin;

      bool onSideA = (m_cacheDir >= Dirs::NORTHEAST && m_cacheDir <= Dirs::SOUTH);
      m_channelEnd.ClaimChannelEnd(channel, onSideA);
    }

    void AssertConnected() const
    {
      if (!m_tile || !m_longlivedLock)
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    /**
     * Gets the Tile that this EventWindow is taking place inside.
     *
     * @returns The Tile that this EventWindow is taking place inside.
     */
    Tile<CC>& GetTile()
    {
      AssertConnected();
      return *m_tile;
    }

    /**
     * Gets the long-lived lock that must be held for us to be on the
     * active side of a cache transaction
     *
     * @returns The LonglivedLock associated with this cache processor
     */
    LonglivedLock& GetLonglivedLock()
    {
      AssertConnected();
      return *m_longlivedLock;
    }

    /**
       Given a full untransformed coordinate in our Tile, compute the
       corresponding position in the full untransformed coordinates of
       our peer's Tile (whether the result is actually within the
       bounds of the peer Tile or not).
     */
    SPoint LocalToRemote(const SPoint & local) const
    {
      AssertConnected();
      return local - m_farSideOrigin;
    }

    /**
       Given a full untransformed coordinate in our peer's Tile,
       compute the corresponding position in the full untransformed
       coordinates of our Tile (whether the result is actually within
       the bounds of our Tile or not).
     */
    SPoint RemoteToLocal(const SPoint & remote) const
    {
      AssertConnected();
      return remote + m_farSideOrigin;
    }

    CacheProcessor()
      : m_tile(0)
      , m_longlivedLock(0)
      , m_cacheDir(0)
      , m_centerRegion((Dir) -1)
      , m_checkOdds(INITIAL_CHECK_ODDS)
      , m_remoteConsistentAtomCount(0)
      , m_useAdaptiveRedundancy(true)
      , m_cpState(IDLE)
      , m_eventCenter(0,0)
      , m_farSideOrigin(0,0)
    { }

  };
} /* namespace MFM */

#include "CacheProcessor.tcc"

#endif /*CACHEPROCESSOR_H*/
