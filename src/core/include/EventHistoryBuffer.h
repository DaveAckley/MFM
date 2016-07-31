/*                                              -*- mode:C++ -*-
  EventHistoryBuffer.h Rolling buffer of rewindable/replayable event changes
  Copyright (C) 2016 The Regents of the University of New Mexico.  All rights reserved.

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
   \file EventHistoryBuffer.h Rolling buffer of rewindable/replayable event changes
   \author David H. Ackley.
   \date (C) 2016 All rights reserved.
   \lgpl
*/
#ifndef EVENTHISTORYBUFFER_H
#define EVENTHISTORYBUFFER_H

#include "EventHistoryItem.h"
#include "Base.h"
#include "Sense.h"
#include "MDist.h"
#include "ByteSink.h"

namespace MFM
{

  template <class EC> class Tile; // FORWARD
  template <class EC> class EventWindow; // FORWARD

  /**
     An EventHistoryBuffer records changes made by recent events in a
     tile, allowing the state of a tile to be rewound and replayed
     within the bounds of the depth of the history buffer.  
  */
  template <class EC>
  class EventHistoryBuffer
  {
  private:
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };
  public:
    enum { 
      SITE_COUNT = EVENT_WINDOW_SITES(R),
      BASE_ATOM,
      SITE_SENSORS,
      BASE_PAINT
    };

    /**
       Printing routine for debug
     */
    void Print(ByteSink & bs) const __attribute__ ((used)) ;

    EventHistoryBuffer(Tile<EC> & forTile, u32 bufferSize, EventHistoryItem * buffer)
      : m_tile(forTile)
      , m_bufferSize(bufferSize)
      , m_historyBuffer(buffer)
      , m_oldestEventStart(0)
      , m_newestEventEnd(1)
      , m_cursor(-1)
      , m_historyActive(true)
      , m_eventsAdded(0)
      , m_itemsInEvent(0)
      , m_makingEvent(false)
      , m_makingEventStart(0)
    {
      MFM_API_ASSERT_NONNULL(buffer);
      MFM_API_ASSERT_ARG(m_bufferSize > 100); // ?? what is safe here, if we always want at least one event in the buffer??
      for (u32 i = 0; i < m_bufferSize; ++i)
        m_historyBuffer[i].MakeUnused();
      // Load up a dummy event to establish the invariants
      m_historyBuffer[m_oldestEventStart].MakeStart(SPoint(0,0), 0);
      m_historyBuffer[m_oldestEventStart].mHeaderItem.m_count = 1;
      m_historyBuffer[m_newestEventEnd].MakeEnd(m_historyBuffer[m_oldestEventStart], 1);
      m_cursor = m_oldestEventStart;
    }

    bool IsCursorAtAnEventEnd() const
    {
      return m_cursor >= 0 && m_historyBuffer[m_cursor].IsEnd();
    }

    bool IsCursorAtAnEventStart() const
    {
      return m_cursor >= 0 && m_historyBuffer[m_cursor].IsStart();
    }

    bool SiteOfCursor(SPoint & ret) const
    {
      if (m_cursor < 0) return false;
      EventHistoryItem & item = m_historyBuffer[m_cursor];
      if (!item.IsHeader()) return false;
      ret = item.GetHeaderSiteInTile();
      return true;
    }

    bool MoveCursor(s32 distance)
    {
      while (distance++ < 0)
        if (!MoveCursorOlder()) return false;
      while (--distance > 0)
        if (!MoveCursorNewer()) return false;
      return true;
    }

    bool MoveCursorToNewest()
    {
      if (m_cursor < 0) return false;
      while (m_cursor != m_newestEventEnd)
      {
        if (!MoveCursorNewer()) return false;
      }
      return true;
    }

    bool MoveCursorToOldest()
    {
      if (m_cursor < 0) return false;
      while (m_cursor != m_oldestEventStart) 
      {
        if (!MoveCursorOlder()) return false;
      }
      return true;
    }

    bool MoveCursorOlder()
    {
      if (m_historyBuffer[m_cursor].IsEnd())
      {
        s32 start = StartOfEventEndedHere(m_cursor);
        if (start < 0) return false;
        SPoint ctr = m_historyBuffer[m_cursor].GetHeaderSiteInTile();
        while ((m_cursor = Decrement(m_cursor)) != start)
        {
          EventHistoryItem & item = m_historyBuffer[m_cursor];
          MFM_API_ASSERT_STATE(item.IsDelta());
          ApplyDelta(false, item.mDeltaItem, m_tile, ctr);
        }
        return true;
      }
      else if (m_historyBuffer[m_cursor].IsStart())
      {
        if ((u32) m_cursor == m_oldestEventStart) 
          return false;
        s32 prevend = GetWrappedIndex(m_cursor - 1);
        MFM_API_ASSERT_STATE(m_historyBuffer[prevend].IsEnd());
        m_cursor = prevend;
        return true;
      }
      return false;
    }

    bool MoveCursorNewer()
    {
      if (m_historyBuffer[m_cursor].IsStart())
      {
        s32 end = EndOfEventStartedHere(m_cursor);
        if (end < 0) return false;
        SPoint ctr = m_historyBuffer[m_cursor].GetHeaderSiteInTile();
        while ((m_cursor = Increment(m_cursor)) != end)
        {
          EventHistoryItem & item = m_historyBuffer[m_cursor];
          MFM_API_ASSERT_STATE(item.IsDelta());
          ApplyDelta(true, item.mDeltaItem, m_tile, ctr);
        }
        return true;
      }
      else if (m_historyBuffer[m_cursor].IsEnd())
      {
        if ((u32) m_cursor == m_newestEventEnd) 
          return false;
        s32 nextstart = Increment(m_cursor);
        MFM_API_ASSERT_STATE(m_historyBuffer[nextstart].IsStart());
        m_cursor = nextstart;
        return true;
      }
      return false;
    }

    bool IsHistoryActive() const { return m_historyActive; }

    void SetHistoryActive(bool active) { m_historyActive = active; }

    /**
       Adds all observable changes associated with \c ew and its tile.
       Used for local events.
     */
    void AddEventWindow(const EventWindow<EC> & ew) ;

    /**
       Begins a new event record.  Used for remote cache events.
    */
    void AddEventStart(const SPoint ctr) ;

    /**
      Adds an atom to an in-progress event record.  Used for remote cache events.
    */
    void AddEventAtom(u32 siteInWindow, const T & oldAtom, const T & newAtom) ;

    /**
       Completes a new event record.  Used for remote cache events.
    */
    void AddEventEnd() ;

    u32 CountEventsInHistory() const ;

  private:

    void ApplyDelta(bool toNewer, EventHistoryItem::DeltaItem & di, Tile<EC>& tile, const SPoint ctr) ;

    // Updates m_itemsInEvent
    void RecordAtomChanges(u32 siteInWindow, const T& oldAtom, const T& newAtom) ;

    // Updates m_itemsInEvent
    void RecordBaseChanges(const Base<AC>& oldBase, const Base<AC>& newBase) ;

    // Updates m_itemsInEvent
    void RecordSensorChanges(const SiteSensors& oldSense, const SiteSensors& newBase) ;

    EventHistoryItem & AllocateNextItem() 
    {
      m_newestEventEnd = GetWrappedIndex(m_newestEventEnd + 1);
      EventHistoryItem & item = m_historyBuffer[m_newestEventEnd];

      if (m_newestEventEnd == m_oldestEventStart)
      {
        MFM_API_ASSERT_STATE(item.IsStart());
        m_oldestEventStart = GetWrappedIndex(m_oldestEventStart + item.GetHeaderItems() + 1);
        MFM_API_ASSERT_STATE(m_historyBuffer[m_newestEventEnd].IsStart());
      }
      return item;
    }

    void DeallocateCurrentStart() 
    {
      EventHistoryItem & item = m_historyBuffer[m_newestEventEnd];
      MFM_API_ASSERT_STATE(item.IsStart());
      item.MakeUnused();
      m_newestEventEnd = GetWrappedIndex(m_newestEventEnd - 1);
    }

    bool StepBackward() ;
    bool StepForward() ;

    Tile<EC> & m_tile;
    u32 m_bufferSize;
    EventHistoryItem * m_historyBuffer;
    u32 m_oldestEventStart;
    u32 m_newestEventEnd;

    /*
      History cursor management:

      if m_cursor < 0, it is invalid and must be reset according to
      mechanisms not defined here.

      Otherwise m_cursor indexes a valid EHI, which must be of type
      END or START.

      If it is an END item, the event it is the end of IS currently
      reflected in the tile state.  GetEventSite

      If it is a START item, the event it is the start of is NOT
      currently reflected in the tile state.

      Stepping the cursor backwards succeeds if there is a valid START
      item strictly prior to the current cursor position. (Whether the
      current cursor position is a START or an END).  If so, then,
      first, the site number of the event is accessed from that START
      item.  Then cursor moves backwards to that START item, writing
      to the tile the 'oldValue' field of all DELTA items found along
      the way, using the site of the event to position the deltas.
      The cursor ends up on that START item.

      Stepping the cursor forwards succeeds if there is a valid END
      item strictly after the current cursor position.  (Whether the
      current cursor position is a START or an END).  If so, the site
      number is accessed from the START item under the cursor, and
      then the cursor moves to that END item, writing to the tile the
      'newValue' field of all DELTA items found along the way, using
      the site of the event to position the deltas.  The cursor ends
      up on that END item.

      If AddEventWindow is called while m_cursor != m_lastUsedItem,
      then before the event is recorded, the following occurs:

      - if m_cursor is at an END item, then m_lastUsedItem is set to
      m_cursor, and m_cursor is set to -1,

      - if m_cursor is at a START item, then m_lastUsedItem is set to
      the index prior to m_cursor, and m_cursor is set to -1.

      This has the effect of truncating the event history at m_cursor,
      when a new event window is added.  (Note m_cursor will be set >=
      0 at the end of AddEventWindow, if a non-trivial event is
      actually stored.)

    */
    s32 m_cursor; // AddEventWindow sets this to index of just-added end item
    bool m_historyActive;
    u32 m_eventsAdded;  // Wrappable rolling count of events ever added to buffer
    u32 m_itemsInEvent;
    bool m_makingEvent;  // true between AddEventStart and AddEventEnd
    u32 m_makingEventStart;  // index of start item of in-progress event

    u32 GetWrappedIndex(s32 index) const
    {
      while (index < 0) index += m_bufferSize;
      while (((u32) index) >= m_bufferSize) index -= m_bufferSize;
      return (u32) index;
    }
    u32 Increment(u32 index) const
    {
      return GetWrappedIndex(index + 1);
    }
    u32 Decrement(u32 index) const
    {
      return GetWrappedIndex(index - 1);
    }

    const EventHistoryItem & GetItemFromWrappedIndex(u32 index) const
    {
      return m_historyBuffer[GetWrappedIndex(index)];
    }

    EventHistoryItem & GetItemFromWrappedIndex(u32 index) 
    {
      return m_historyBuffer[GetWrappedIndex(index)];
    }

    /**
       Returns the start item index (which is >= 0), if \c endIndex is
       at an END item that has a corresponding START item, else -1.
    */
    s32 StartOfEventEndedHere(s32 endIndex) const
    {
      const EventHistoryItem & ehiEnd = GetItemFromWrappedIndex(endIndex);
      if (!ehiEnd.IsEnd()) return -1;

      s32 startIndex = endIndex - (s32) ehiEnd.GetHeaderItems();
      const EventHistoryItem & ehiStart = GetItemFromWrappedIndex(startIndex);
    
      if (!ehiStart.IsStart() || ehiEnd.GetHeaderEventNumber() != ehiStart.GetHeaderEventNumber())
        return -1;
    
      return startIndex;
    }

    /**
       Returns the end item index (which is >= 0), if \c startIndex is
       at a START item that has a corresponding END item, else -1.
    */
    s32 EndOfEventStartedHere(s32 startIndex) const
    {
      const EventHistoryItem & ehiStart = GetItemFromWrappedIndex(startIndex);
      if (!ehiStart.IsStart()) return -1;

      s32 endIndex = startIndex + ehiStart.GetHeaderItems();
      const EventHistoryItem & ehiEnd = GetItemFromWrappedIndex(endIndex);
    
      if (!ehiEnd.IsEnd() || ehiEnd.GetHeaderEventNumber() != ehiStart.GetHeaderEventNumber())
        return -1;
    
      return endIndex;
    }

  };

} /* namespace MFM */

#include "EventHistoryBuffer.tcc"

#endif /*EVENTHISTORYBUFFER_H*/
