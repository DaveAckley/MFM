/*                                              -*- mode:C++ -*-
  ParamConfig.h Support for modularized compile-time parameters
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
  \file ParamConfig.h Support for modularized compile-time parameters
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PARAMCONFIG_H
#define PARAMCONFIG_H

#include "itype.h"

namespace MFM {

  /**
   * A template class used to hold all compile-time parameters for an
   * MFM core, except the type of the Atom.  Template defaults are P1
   * standard or typical usage.
   */
  template <u32 L = 64, // Bits per atom
            u32 R = 4,  // Event window radius
            u32 B = 8,  // Element table bits
            u32 W = 40, // Tile width
            u32 EDS = 4 // Per-Tile element data slots for experimenter's use
            >
  struct ParamConfig {

    /**
     * BITS_PER_ATOM is the total amount of storage per atom for this
     * ParamConfiguration.  Note this value must be compatible with
     * the expectations of the atom type chosen in the CoreConfig
     * template.
     */
    enum { BITS_PER_ATOM = L };

    /**
     * EVENT_WINDOW_RADIUS is the size of the event window for this
     * ParamConfiguration.
     */
    enum { EVENT_WINDOW_RADIUS = R };

    /**
     * ELEMENT_TABLE_BITS is the log2 of the size of the element table
     * for this ParamConfiguration.
     */
    enum { ELEMENT_TABLE_BITS = B };

    /**
     * TILE_WIDTH is the number of sites on a side of the tile for
     * this ParamConfiguration.
     */
    enum { TILE_WIDTH = W };

    /**
     * ELEMENT_DATA_SLOTS is the count of u64's to allocate in each
     * ElementTable to hold arbitrary element-specific data , for
     * purposes of experimental data storage, statistics gathering,
     * and so forth.  Key point is that since a separate ElementTable
     * goes in each Tile, and each Tile has its own thread, these
     * slots are likewise available per-thread.
     */
    enum { ELEMENT_DATA_SLOTS = EDS };

  };

} /* namespace MFM */

#endif /*PARAMCONFIG_H*/
