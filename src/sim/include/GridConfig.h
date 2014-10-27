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

namespace MFM {

  /**
   * A template class used to hold all compile-time GridConfiguration
   * parameters for a MFM.  We make all MFM template classes depend on
   * this one type, rather than having each depend on whichever
   * compile-time parameters they reference.
   *
   * If an additional compile-time parameter is needed, rather than
   * having to add it to thousands of template declarations throughout
   * the codebase, we just add it here, and then modify only the
   * template classes that need to use the new parameter.
   */
  template <class CC,      // CoreConfig
            u32 W = 5,     // Grid width in tiles
            u32 H = 3>     // Grid height in tiles
  struct GridConfig {

    /**
     * CORE_CONFIG is the Config type for the core MFM parameters.
     */
    typedef CC CORE_CONFIG;

    /**
     * GRID_WIDTH is the number of columns of tiles in this Grid
     * Configuration.
     */
    enum { GRID_WIDTH = W };

    /**
     * GRID_HEIGHT is the number of rows of tiles in this Grid
     * Configuration.
     */
    enum { GRID_HEIGHT = H };

  };

} /* namespace MFM */

#endif /*GRIDCONFIG_H*/
