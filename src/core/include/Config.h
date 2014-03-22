/*                                              -*- mode:C++ -*-
  Config.h Support for modularized compile-time configuration parameters
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
  \file Config.h Support for modularized compile-time configuration parameters
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "itype.h"

namespace MFM {

  /**
   * A template class used to hold all compile-time configuration
   * parameters for a MFM.  We make all MFM template classes depend on
   * this one type, rather than having each depend on whichever
   * compile-time parameters they reference.
   *
   * If an additional compile-time parameter is needed, rather than
   * having to add it to thousands of template declarations throughout
   * the codebase, we just add it here, and then modify only the
   * template classes that need to use the new parameter.
   */
  template <class T, u32 R = 4, u32 B=8, u32 W = 40>
  struct Config {

    /**
     * ATOM_TYPE is the subclass of Atom for this configuration.
     */
    typedef T ATOM_TYPE;

    /**
     * EVENT_WINDOW_RADIUS is the size of the event window for this
     * configuration.
     */
    enum { EVENT_WINDOW_RADIUS = R };

    /**
     * ELEMENT_TABLE_BITS is the log2 of the size of the element table
     * for this configuration.
     */
    enum { ELEMENT_TABLE_BITS = B };

    /**
     * TILE_WIDTH is the number of sites on a side of the tile for
     * this configuration.
     */
    enum { TILE_WIDTH = W };
  };

} /* namespace MFM */

#endif /*CONFIG_H*/
