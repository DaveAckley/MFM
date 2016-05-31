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

#include "EventWindow.h"
#include "EventHistoryItem.h"

namespace MFM
{

  template <class EC> class Tile; // FORWARD

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

    EventHistoryBuffer(Tile<EC> & forTile, u32 bufferSize, EventHistoryItem * buffer)
      : m_tile(forTile)
      , m_bufferSize(bufferSize)
      , m_historyBuffer(buffer)
      , m_lastUsedItem(m_bufferSize - 1)
      , m_historyActive(true)
    {
      MFM_API_ASSERT_NONNULL(buffer);
      MFM_API_ASSERT_ARG(m_bufferSize > 0);
      for (u32 i = 0; i < m_bufferSize; ++i)
        m_historyBuffer[i].MakeUnused();
    }

    bool IsHistoryActive() const { return m_historyActive; }

    void SetHistoryActive(bool active) { m_historyActive = active; }

    void AddEventWindow(const EventWindow<EC> & ew) ;

  private:

    // Returns resulting itemsInEvent
    u32 RecordAtomChanges(u32 siteInWindow, const T& oldAtom, const T& newAtom, u32 itemsInEvent) ;

    // Returns resulting itemsInEvent
    u32 RecordBaseChanges(const Base<AC>& oldBase, const Base<AC>& newBase, u32 itemsInEvent) ;

    // Returns resulting itemsInEvent
    u32 RecordSensorChanges(const SiteSensors& oldSense, const SiteSensors& newBase, u32 itemsInEvent) ;

    EventHistoryItem & AllocateNextItem() 
    {
      if (++m_lastUsedItem >= m_bufferSize)
        m_lastUsedItem = 0;
      return m_historyBuffer[m_lastUsedItem];
    }

    void DeallocateCurrentItem() 
    {
      m_historyBuffer[m_lastUsedItem].MakeUnused();
      if (m_lastUsedItem == 0)
        m_lastUsedItem = m_bufferSize - 1;
      else
        --m_lastUsedItem;
    }

    Tile<EC> & m_tile;
    u32 m_bufferSize;
    EventHistoryItem * m_historyBuffer;
    u32 m_lastUsedItem;
    bool m_historyActive;

  };
} /* namespace MFM */

#include "EventHistoryBuffer.tcc"

#endif /*EVENTHISTORYBUFFER_H*/
