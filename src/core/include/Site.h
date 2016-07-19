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
#include "AtomSerializer.h"

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
    u64 m_lastChangedEventCount;  // in units of Site event count
    u64 m_lastEventNumber;        // in units of total tile events
    bool m_isLiveSite;

  public:
    Site()
      : m_eventCount(0)
      , m_lastChangedEventCount(0)
      , m_lastEventNumber(0)
      , m_isLiveSite(true)
    { }

    void RecordEventAtSite(u64 eventNumber)
    {
      ++m_eventCount;
      m_lastEventNumber = eventNumber;
    }

    void SaveConfig(ByteSink& bs, AtomTypeFormatter<AC> & atf) const
    {
      bs.Printf(",%D", m_isLiveSite);
      // 64 bit stuff not yet exposed via Printf..
      bs.Print(m_eventCount, Format::LXX64);
      bs.Print(m_lastChangedEventCount, Format::LXX64);
      bs.Print(m_lastEventNumber, Format::LXX64);

      {
        T tmp = m_atom;
        bs.Printf(",");
        atf.PrintAtomType(tmp, bs);
        AtomSerializer<AC> as(tmp);
        bs.Printf(",%@", &as);
      }

      m_base.SaveConfig(bs, atf);
    }

    bool LoadConfig(LineCountingByteSource& bs, AtomTypeFormatter<AC> & atf)
    {
      u32 tmp_m_isLiveSite;
      if (2 != bs.Scanf(",%D", &tmp_m_isLiveSite)) return false;

      u64 tmp_m_eventCount;
      u64 tmp_m_lastChangedEventCount;
      u64 tmp_m_lastEventNumber;

      // 64 bit stuff not yet exposed via Scanf..
      if (!bs.Scan(tmp_m_eventCount, Format::LXX64)) return false;
      if (!bs.Scan(tmp_m_lastChangedEventCount, Format::LXX64)) return false;
      if (!bs.Scan(tmp_m_lastEventNumber, Format::LXX64)) return false;

      // Arrgh, this code is hideous AND copy-pasted from Base.h :( XXX
      if (1 != bs.Scanf(",")) return false;

      T defaultAtom;
      {
        if (!atf.ParseAtomType(bs, defaultAtom)) // All we care about is the type..
          return false;

        T tmp_m_atom;
        AtomSerializer<AC> as2(tmp_m_atom);
        if (2 != bs.Scanf(",%@", &as2))
          return false;

        for (u32 i = T::ATOM_FIRST_STATE_BIT; i < T::BPA; ++i) // merge and pray.  jus'tryin't'elp
        {
          defaultAtom.GetBits().StoreBit(i, tmp_m_atom.GetBits().ReadBit(i));
        }
      }

      if (!m_base.LoadConfig(bs, atf))
        return false;

      m_atom = defaultAtom;

      m_isLiveSite = tmp_m_isLiveSite;
      m_eventCount = tmp_m_eventCount;
      m_lastChangedEventCount = tmp_m_lastChangedEventCount;
      m_lastEventNumber = tmp_m_lastEventNumber;

      return true;
    }

    void Sense(SiteTouchType stt)
    {
      m_base.GetSensory().Touch(stt, m_eventCount);
    }

    bool InRecentProximity() const
    {
      return TOUCH_TYPE_PROXIMITY == RecentTouch();
    }

    u32 RecentTouch() const
    {
      return m_base.GetSensory().RecentTouch(m_eventCount);
    }

    bool HasRecentLightTouch()
    {
      return TOUCH_TYPE_LIGHT == RecentTouch();
    }

    void PutAtom(const T & newAtom) { m_atom = newAtom; }
    T & GetAtom() { return m_atom; }
    const T & GetAtom() const { return m_atom; }

    Base<AC> & GetBase() { return m_base; }
    const Base<AC> & GetBase() const { return m_base; }

    u32 GetPaint() const {
      return GetBase().GetPaint();
    }

    void SetPaint(u32 paint) {
      GetBase().SetPaint(paint);
    }

    void Clear() {
      m_atom.SetEmpty();
      m_eventCount = 0;
      m_lastChangedEventCount = 0;
      m_base.GetSensory().Clear();
    }

    u64 GetEventCount() const {
      return m_eventCount;
    }

    u64 GetLastChangedEventCount() const {
      return m_lastChangedEventCount;
    }

    void MarkChanged() {
      m_lastChangedEventCount = m_eventCount;
    }

    u64 GetWriteAge() const {
      return m_eventCount - m_lastChangedEventCount;
    }

    u64 GetEventAge(u64 currentEventNumber) const {
      return m_lastEventNumber - currentEventNumber;
    }

  };
} /* namespace MFM */

#endif /*SITE_H*/
