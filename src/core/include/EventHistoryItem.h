/*                                              -*- mode:C++ -*-
  EventHistoryItem.h Component of buffered event recording
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
  \file EventHistoryItem.h Component of buffered event recording
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \lgpl
 */
#ifndef EVENTHISTORYITEM_H
#define EVENTHISTORYITEM_H

#include "itype.h"
#include "Point.h"
#include "Fail.h"

namespace MFM
{
  /**
     An EventHistoryItem is a semi-compact description of a change
     caused by an event.  It is in effect a union of the different
     components of an event -- event start, source, and end; old value
     at position; etc -- and a rolling buffer of EventHistoryItems can
     be 'rewound' from the current position to reconstruct some number
     of initial states of previous events, depending on the size of
     the buffer and the amount of change the events caused.

     Each item is 12 bytes long; the first byte is always a type code.

   */
  union EventHistoryItem
  {
    enum Type {
      START,       //> Beginning of an event record
      DELTA,       //> Change record with an event history 
      END,         //> End of an event record
      UNUSED       //> Initial state of buffer entries
    };

    u8 m_type;                  // alias for m_type in each item type

    struct HeaderItem {
      u8 m_type;                // ==START or END
      u8 m_count;               // Total items in this event excluding START but including END
      u8 m_pad[2];              // (unused)
      s16 m_siteInTileX;        // Absolute X pos of event in tile (can be negative for caches)
      s16 m_siteInTileY;        // Absolute Y pos of event in tile
      u32 m_eventNumber;        // Number of this event in history
    } mHeaderItem;

    struct DeltaItem {
      u8 m_type;                // ==DELTA
      u8 m_site;                // site in window or base code
      u8 m_count;               // item number in this event
      u8 m_word;                // word in atom or storage
      u32 m_oldValue;           // To rewind
      u32 m_newValue;           // To replay
    } mDeltaItem;

    void MakeUnused()
    {
      m_type = UNUSED;
    }

    void MakeStart(const SPoint siteInTile, u32 eventNumber) 
    {
      HeaderItem & s = this->mHeaderItem;
      s.m_type = START;
      s.m_siteInTileX = siteInTile.GetX();
      s.m_siteInTileY = siteInTile.GetY();
      s.m_eventNumber = eventNumber;
    }

    void MakeEnd(const EventHistoryItem & startItem, u32 itemsInEvent) 
    {
      MFM_API_ASSERT_ARG(startItem.IsStart());
      const HeaderItem & start = startItem.mHeaderItem;
      HeaderItem & e = this->mHeaderItem;
      e.m_type = END;
      e.m_siteInTileX = start.m_siteInTileX;
      e.m_siteInTileY = start.m_siteInTileY;
      e.m_count = itemsInEvent; 
      e.m_eventNumber = start.m_eventNumber;
    }

    void MakeDelta(u32 siteInWindow, u32 itemInEvent, u32 wordInAtom, u32 oldv, u32 newv) 
    {
      DeltaItem & d = this->mDeltaItem;
      d.m_type = DELTA;
      d.m_site = siteInWindow;
      d.m_count = itemInEvent;
      d.m_word = wordInAtom;
      d.m_oldValue = oldv;
      d.m_newValue = newv;
    }

    u32 GetHeaderEventNumber() const 
    {
      MFM_API_ASSERT_STATE(IsType(START) || IsType(END));
      const HeaderItem & s = this->mHeaderItem;
      return s.m_eventNumber;
    }

    SPoint GetHeaderSiteInTile() const 
    {
      MFM_API_ASSERT_STATE(IsType(START) || IsType(END));
      const HeaderItem & s = this->mHeaderItem;
      return SPoint(s.m_siteInTileX, s.m_siteInTileY);
    }

    u32 GetHeaderItems() const 
    {
      MFM_API_ASSERT_STATE(IsType(START) || IsType(END));
      const HeaderItem & e = this->mHeaderItem;
      return e.m_count;
    }

    void Print(ByteSink & bs) const;

    Type GetType() const { return (Type) this->m_type; }
    const char * GetTypeName() const
    {
      switch (GetType()) {
      case START: return "START";
      case DELTA: return "DELTA";
      case END:   return "END";
      case UNUSED: return "UNUSED";
      default: return "INVAL?";
      }
    }

    bool IsEnd() const { return IsType(END); }
    bool IsStart() const { return IsType(START); }
    bool IsDelta() const { return IsType(DELTA); }
    bool IsUnused() const { return IsType(UNUSED); }
    bool IsHeader() const { return IsStart() || IsEnd(); }

    bool IsType(Type type) const
    {
      return this->m_type == type;
    }
  };
} /* namespace MFM */

#include "EventHistoryItem.tcc"

#endif /*EVENTHISTORYITEM_H*/
