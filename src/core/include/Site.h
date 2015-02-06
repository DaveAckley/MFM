/*                                              -*- mode:C++ -*-
  Site.h A location for a single atom
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
  \file Site.h A location for a single atom
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
    u64 m_eventCount;
    s64 m_lastEventEventNumber;
    u64 m_lastChangedEventNumber;
    T m_atom;
    bool m_isLiveSite;

  public:
    T & GetAtom() { return m_atom; }

    const T & GetAtom() const { return m_atom; }

    void PutAtom(const T & newAtom) {
      m_atom = newAtom;
    }

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
