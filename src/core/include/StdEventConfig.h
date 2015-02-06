/*                                              -*- mode:C++ -*-
  StdEventConfig.h Default compile-time parameters up to the event level
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
  \file StdEventConfig.h Default compile-time parameters up to the event level
  \author David H. Ackley.
  \date (C) 2014, 2015 All rights reserved.
  \lgpl
 */
#ifndef STDEVENTCONFIG_H
#define STDEVENTCONFIG_H

#include "AtomConfig.h"
#include "EventConfig.h"
#include "Site.h"
#include "P3Atom.h"

namespace MFM {
  typedef P3Atom StdAtom;
  typedef Site<P3AtomConfig> StdSite;
  typedef EventConfig<StdSite, 4> StdEventConfig;
}

#endif /* STDEVENTCONFIG_H */
