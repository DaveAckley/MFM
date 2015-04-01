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

namespace MFM
{

  /**
     An Site provides a single Atom plus all information associated
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
    T m_state;
    T m_base;
    u64 m_eventCount;
    u64 m_lastChangedEventNumber;
    s64 m_lastEventEventNumber;
    u32 m_rgbIn;
    u32 m_rgbOut;
    u32 m_senseIn;
    u32 m_motorOut;
    bool m_isLiveSite;

  public:
    Site()
      : m_eventCount(0)
      , m_lastChangedEventNumber(0)
      , m_lastEventEventNumber(0)
      , m_rgbIn(0)
      , m_rgbOut(0)
      , m_senseIn(0)
      , m_motorOut(0)
      , m_isLiveSite(true)
    { }

    u32 GetRGBIn() const { return m_rgbIn; }
    void SetRGBIn(u32 newRGB) { m_rgbIn = newRGB; }

    u32 GetRGBOut() const { return m_rgbOut; }
    void SetRGBOut(u32 newRGB) { m_rgbOut = newRGB; }

    u32 GetSenseIn() const { return m_senseIn; }
    void SetSenseIn(u32 newSense) { m_senseIn = newSense; }

    u32 GetMotorOut() const { return m_motorOut; }
    void SetMotorOut(u32 newMotor) { m_motorOut = newMotor; }

    void PutAtom(const T & newAtom) { m_atom = newAtom; }
    T & GetAtom() { return m_atom; }
    const T & GetAtom() const { return m_atom; }

    void PutStateAtom(const T & newState) { m_state = newState; }
    T & GetStateAtom() { return m_state; }
    const T & GetStateAtom() const { return m_state; }

    void PutBaseAtom(const T & newBase) { m_base = newBase; }
    T & GetBaseAtom() { return m_base; }
    const T & GetBaseAtom() const { return m_base; }

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
    }

  };
} /* namespace MFM */

#endif /*SITE_H*/
