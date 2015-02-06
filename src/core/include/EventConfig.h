/*                                              -*- mode:C++ -*-
  EventConfig.h The compile-time parameters at the event level
  Copyright (C) 2014, 2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file EventConfig.h The compile-time parameters at the event level
  \author David H. Ackley.
  \date (C) 2014, 2015 All rights reserved.
  \lgpl
 */
#ifndef EVENTCONFIG_H
#define EVENTCONFIG_H

#include "itype.h"
#include "AtomConfig.h"

namespace MFM {

  /**
   * A template class used to hold compile-time parameters relevant to
   * a single event for an MFM Element.
   */
  template <class S,    // A Site
            u32 R       // An event window radius
            >
  struct EventConfig {
    /**
     * SITE is the type of Site used in this EventConfig.
     */
    typedef S SITE;

    /**
     * ATOM_CONFIG is the AtomConfig used in the Site in this EventConfig.
     */
    typedef typename SITE::ATOM_CONFIG ATOM_CONFIG;

#if 0 // Let's not pull this up; it can be confusing.  Users should drill down to AC for it
    /**
     * T is the type of Atom used in this EventConfig.
     */
    typedef typename ATOM_CONFIG::ATOM_TYPE T;
#endif

    /**
     * EVENT_WINDOW_RADIUS is the size of the event window for this
     * EventConfig.
     */
    enum { EVENT_WINDOW_RADIUS = R };

  };

} /* namespace MFM */

#endif /*EVENTCONFIG_H*/
