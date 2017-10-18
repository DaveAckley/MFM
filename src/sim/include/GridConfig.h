/*                                              -*- mode:C++ -*-
  GridConfig.h Support for modularized compile-time grid parameters
  Copyright (C) 2014,2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2017 Ackleyshack,LLC.  All rights reserved.

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
  \author Elena S. Ackley.
  \date (C) 2014, 2016, 2017 All rights reserved.
  \lgpl
 */
#ifndef GRIDCONFIG_H
#define GRIDCONFIG_H

#include "itype.h"
#include "EventConfig.h"
#include "Tile.h"   /* for GridLayoutPattern */

namespace MFM {

  /**
   * A template class used to hold compile-time parameters for an MFM
   * at the Tile-to-Grid level.
   */
  template <class EC,      // EventConfig
            u32 WIDTH,
	    u32 HEIGHT,
            u32 HISTORYSIZE>
  struct GridConfig {

    /**
     * EVENT_CONFIG is the Config type for the core MFM parameters.
     */
    typedef EC EVENT_CONFIG;

    /**
     * TILE_WIDTH is the number of sites wide for a tile in
     * this GridConfig
     */
    enum { TILE_WIDTH = WIDTH };

    /**
     * TILE_HEIGHT is the number of sites high for a tile in
     * this GridConfig
     */
    enum { TILE_HEIGHT = HEIGHT };

    /**
     * EVENT_HISTORY_SIZE is the number of EventHistoryItems that a
     * tile remembers in this GridConfig
     */
    enum { EVENT_HISTORY_SIZE = HISTORYSIZE };

    /**
     * OWNED_WIDTH is the number of sites wide for a tile in
     * this GridConfig, excluding the caches
     */
    enum { OWNED_WIDTH = TILE_WIDTH - 2 * EC::EVENT_WINDOW_RADIUS };

    /**
     * OWNED_HEIGHT is the number of sites high for a tile in
     * this GridConfig, excluding the caches
     */
    enum { OWNED_HEIGHT = TILE_HEIGHT - 2 * EC::EVENT_WINDOW_RADIUS };


  };

} /* namespace MFM */

#endif /*GRIDCONFIG_H*/
