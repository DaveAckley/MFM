/*                                              -*- mode:C++ -*-
  Site.h A location for a single atom and associated state
  Copyright (C) 2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file Site.h A location for a single atom and associated state
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef SITE_H
#define SITE_H

#include "itype.h"
#include "AtomConfig.h"
#include "Base.h"

namespace MFM
{

  /**
     A Site holds a Base and an Atom, and all information associated
     with that Atom, such as access times, ages, and so forth.  It is
     a template depending only on an AtomConfig (AC).
   */
  template <class AC>
  class Site
  {
  public:
    /**
       Present the AtomConfig in use
     */
    typedef AC ATOM_CONFIG;

    // Extract short names for parameter types
    typedef typename ATOM_CONFIG::ATOM_TYPE T;

  private:
    T m_atom;
    Base<AC> m_base;
    u64 m_eventCount;
    u64 m_lastChangedEventNumber;
    s64 m_lastEventEventNumber;
    bool m_isLiveSite;

  public:
    Site()
      : m_eventCount(0)
      , m_lastChangedEventNumber(0)
      , m_lastEventEventNumber(S32_MIN) // init deep in past
      , m_isLiveSite(true)
    { }

    void SaveConfig(ByteSink& bs) const
    {
      bs.Printf(",%D", m_isLiveSite);
      // 64 bit stuff not yet exposed via Printf..
      bs.Print(m_eventCount, Format::LXX64);
      bs.Print(m_lastChangedEventNumber, Format::LXX64);
      bs.Print(m_lastEventEventNumber, Format::LXX64);
      m_base.SaveConfig(bs);
    }

    bool LoadConfig(LineCountingByteSource& bs)
    {
      u32 tmp_m_isLiveSite;
      if (2 != bs.Scanf(",%D", &tmp_m_isLiveSite)) return false;

      u64 tmp_m_eventCount;
      u64 tmp_m_lastChangedEventNumber;
      u64 tmp_m_lastEventEventNumber;

      // 64 bit stuff not yet exposed via Scanf..
      if (!bs.Scan(tmp_m_eventCount, Format::LXX64)) return false;
      if (!bs.Scan(tmp_m_lastChangedEventNumber, Format::LXX64)) return false;
      if (!bs.Scan(tmp_m_lastEventEventNumber, Format::LXX64)) return false;

      if (!m_base.LoadConfig(bs))
        return false;

      m_isLiveSite = tmp_m_isLiveSite;
      m_eventCount = tmp_m_eventCount;
      m_lastChangedEventNumber = tmp_m_lastChangedEventNumber;
      m_lastEventEventNumber = tmp_m_lastEventEventNumber;
      return true;
    }

    void Sense(SiteTouchType stt)
    {
      m_base.GetSensory().Touch(stt, m_lastEventEventNumber);
    }

    bool InRecentProximity()
    {
      return TOUCH_TYPE_PROXIMITY == m_base.GetSensory().RecentTouch(m_lastEventEventNumber);
    }

    bool HasRecentLightTouch()
    {
      return TOUCH_TYPE_LIGHT == m_base.GetSensory().RecentTouch(m_lastEventEventNumber);
    }

    void PutAtom(const T & newAtom) { m_atom = newAtom; }
    T & GetAtom() { return m_atom; }
    const T & GetAtom() const { return m_atom; }

    Base<AC> & GetBase() { return m_base; }
    const Base<AC> & GetBase() const { return m_base; }

    void Clear() {
      m_atom.SetEmpty();
      m_eventCount = 0;
      m_lastChangedEventNumber = 0;

      // Set last event deep in the past to avoid initial color
      m_lastEventEventNumber = S32_MIN; // (stored into s64..)
    }

    u64 GetEventCount() const {
      return m_eventCount;
    }

    u64 GetLastChangedEventNumber() const {
      return m_lastChangedEventNumber;
    }

    void SetLastChangedEventNumber(u64 eventNumber) {
      m_lastChangedEventNumber = eventNumber;
    }

    s64 GetLastEventEventNumber() const {
      return m_lastEventEventNumber;
    }

    void SetLastEventEventNumber(u64 eventNumber) {
      m_lastEventEventNumber = (s64) eventNumber;
      m_base.SetLastEventEventNumber(eventNumber);
    }

  };
} /* namespace MFM */

#endif /*SITE_H*/
