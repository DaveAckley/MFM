/*                                              -*- mode:C++ -*-
  Version.h Maintain the version of the MFMv2 code base
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
  \file Version.h Maintain the version of the MFMv2 code base
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef VERSION_H
#define VERSION_H

namespace MFM
{
  /**
   * A string representing an abbreviated version number of this
   * instance of the Movable Feast Machine.
   */
  extern const char * MFM_VERSION_STRING_SHORT;

  /**
   * A string representing the extended version number of this
   * instance of the Movable Feast Machine.
   */
  extern const char * MFM_VERSION_STRING_LONG;

} /* namespace MFM */

#endif /*VERSION_H*/
