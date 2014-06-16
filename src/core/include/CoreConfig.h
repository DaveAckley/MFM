/*                                              -*- mode:C++ -*-
  CoreConfig.h Support for modularized compile-time parameters
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
  \file CoreConfig.h Support for modularized compile-time parameters
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CORECONFIG_H
#define CORECONFIG_H

#include "itype.h"

namespace MFM {

  /**
   * A template class used to hold all compile-time parameters needed
   * for compilations in the MFM core.
   */
  template <class A,  // An Atom type
            class P>  // A ParamConfig
  struct CoreConfig {

    /**
     * ATOM_TYPE is the type of the atom for this CoreConfiguration.
     */
    typedef A ATOM_TYPE;

    /**
     * PARAM_CONFIG is the type of the parameters configuration for
     * this CoreConfiguration.
     */
    typedef P PARAM_CONFIG;

  };

} /* namespace MFM */

#endif /*CORECONFIG_H*/
