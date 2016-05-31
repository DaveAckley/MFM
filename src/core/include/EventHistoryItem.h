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

     It is sixteen bytes (using the P3Atom) and the first byte is
     always a type code.

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

    struct StartItem {
      u8 m_type;                // ==START 
      u8 m_siteInTileHigh;      // Really u24 siteInTile
      u16 m_siteInTileLow;
      u32 m_windowCountHigh;    // Top 32 bits of EventWindowsAttempted
      u32 m_windowCountLow;     // Bottom 32 bits of EventWindowsAttempted
    } mStartItem;

    struct EndItem {
      u8 m_type;                // ==END
      u8 m_count;               // Total items in this event excluding START but including END
      u8 m_pad[2];              // (unused)
      u32 m_windowCountHigh;    // Top 32 bits of EventWindowsAttempted
      u32 m_windowCountLow;     // Bottom 32 bits of EventWindowsAttempted
    } mEndItem;

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

    void MakeStart(u32 siteInTile, u64 count) 
    {
      const u32 SITE_BITS = 24;
      const u32 SITE_MASK = (1<<SITE_BITS)-1;

      const u32 SITE_LOW_BITS = 16;
      const u32 SITE_LOW_MASK = (1<<SITE_LOW_BITS)-1;

      MFM_API_ASSERT_ARG((siteInTile & ~SITE_MASK) == 0);

      StartItem & s = this->mStartItem;
      s.m_type = START;
      s.m_siteInTileHigh = siteInTile >> SITE_LOW_BITS;
      s.m_siteInTileLow = siteInTile & SITE_LOW_MASK;
      s.m_windowCountLow = (u32) (count>>0);
      s.m_windowCountHigh = (u32) (count>>32);
    }

    void MakeEnd(u32 itemsInEvent, u64 windowcount) 
    {
      EndItem & e = this->mEndItem;
      e.m_type = END;
      e.m_count = itemsInEvent; 
      e.m_windowCountLow = (u32) (windowcount>>0);
      e.m_windowCountHigh = (u32) (windowcount>>32);
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

  };
} /* namespace MFM */

#include "EventHistoryItem.tcc"

#endif /*EVENTHISTORYITEM_H*/
