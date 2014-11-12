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

    enum {
      /**
         MAX_CHECK_ODDS is the maximum value of m_checkOdds.
         1-in-MAX_CHECK_ODDS is the \e minimum amount of redundancy
         added to packet transfers to monitor remote cache quality.
         If a cache failure is detected, m_checkOdds will be reduced
         below this value, causing the \e actual redundancy to be
         higher, at least temporarily.
       */
      MAX_CHECK_ODDS = 25,

      /**
         INITIAL_CHECK_ODDS is the initial value of m_checkOdds.
         \sa MAX_CHECK_ODDS
         \sa m_checkOdds
       */
      INITIAL_CHECK_ODDS = 10
    };

    /**
       The current odds of including redundant check packets in the
       cache update stream.  Atoms that did \e not change during an
       event will also be transmitted, using type PACKET_CHECK, with
       odds of 1-in-this.  Minimum value is 1, meaning all such
       packets will be transmitted; maximum value is MAX_CHECK_ODDS.
     */
    u32 m_checkOdds;

    u32 GetCheckOdds() const
    {
      return m_checkOdds;
    }

    void ReportCheckFailure()
    {
      if (m_checkOdds > 5)
      {
        m_checkOdds /= 3;
      }
      else
      {
        m_checkOdds = 1;
      }
    }

    void ReportCleanUpdate()
    {
      if (m_checkOdds < MAX_CHECK_ODDS)
      {
        ++m_checkOdds;
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
      MFM_LOG_DBG6(("CP %s (%d,%d): %s->%s",
                    m_tile->GetLabel(),
                    m_farSideOrigin.GetX(),
                    m_farSideOrigin.GetY(),
                    GetStateName(m_cpState),
                    GetStateName(state)));
      m_cpState = state;
    }

  public:

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

    bool IsIdle()
    {
      return m_cpState == IDLE;
    }

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

    bool TryLock()
    {
      return m_channelEnd.TryLock();
    }

    void Unlock()
    {
      m_channelEnd.Unlock();
    }

    bool IsConnected() const
    {
      return m_channelEnd.IsConnected();
    }

    void ClaimCacheProcessor(Tile<CC>& tile, AbstractChannel& channel, bool onSideA, SPoint remoteOrigin)
    {
      if (m_tile)
      {
        FAIL(ILLEGAL_STATE);
      }

      m_tile = &tile;
      m_farSideOrigin = remoteOrigin;
      m_channelEnd.ClaimChannelEnd(channel, onSideA);
    }

    void AssertConnected() const
    {
      if (!m_tile)
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
      , m_checkOdds(INITIAL_CHECK_ODDS)
      , m_cpState(IDLE)
      , m_eventCenter(0,0)
      , m_farSideOrigin(0,0)
    { }

  };
} /* namespace MFM */

#include "CacheProcessor.tcc"

#endif /*CACHEPROCESSOR_H*/
