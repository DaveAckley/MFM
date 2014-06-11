/*                                              -*- mode:C++ -*-
  ExternalConfigFunctions.h Function call objects for external configuration files
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
  \file ExternalConfigFunctions.h Function call objects for external configuration files
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIGFUNCTIONS_H
#define EXTERNALCONFIGFUNCTIONS_H

#include "ExternalConfig.h"

namespace MFM
{
  template <class GC>
  void RegisterExternalConfigFunctions(ExternalConfig<GC> & ec) ;
}

#include "ExternalConfigFunctions.tcc"

#endif /* EXTERNALCONFIGFUNCTIONS_H */
