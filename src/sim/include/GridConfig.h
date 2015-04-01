/*                                              -*- mode:C++ -*-
  GridConfig.h Support for modularized compile-time grid parameters
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
  \file GridConfig.h Support for modularized compile-time grid parameters
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef GRIDCONFIG_H
#define GRIDCONFIG_H

#include "itype.h"
#include "EventConfig.h"

namespace MFM {

  /**
   * A template class used to hold compile-time parameters for an MFM
   * at the Tile-to-Grid level.
   */
  template <class EC,      // EventConfig
            u32 SIDE>
  struct GridConfig {

    /**
     * EVENT_CONFIG is the Config type for the core MFM parameters.
     */
    typedef EC EVENT_CONFIG;

#if 0
    /**
     * GRID_WIDTH is the number of columns of tiles in this Grid
     * Configuration.
     */
    enum { GRID_WIDTH = W };

    /**
     * GRID_HEIGHT is the number of rows of tiles in this Grid
     * Config.
     */
    enum { GRID_HEIGHT = H };
#endif

    /**
     * TILE_SIDE is the number of sites wide (and high) for a tile in
     * this GridConfig
     */
    enum { TILE_SIDE = SIDE };

    /**
     * OWNED_SIDE is the number of sites wide (and high) for a tile in
     * this GridConfig, excluding the caches
     */
    enum { OWNED_SIDE = TILE_SIDE - 2 * EC::EVENT_WINDOW_RADIUS };

  };

} /* namespace MFM */

#endif /*GRIDCONFIG_H*/
