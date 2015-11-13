/*                                              -*- mode:C++ -*-
  AtomConfig.h Support for modularized compile-time parameters
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
  \file AtomConfig.h Support for modularized compile-time parameters
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ATOMCONFIG_H
#define ATOMCONFIG_H

namespace MFM {

  /**
     AtomConfig is a documentation-only base class describing the
     responsibilities of subclasses of Atom (q.v.)
  */
  struct AtomConfig {

    /**
      ATOM_TYPE is the type of the atom for this AtomConfiguration.
     */
    // typedef My99Atom ATOM_TYPE;

    /**
      ATOM_CATEGORY is the assigned basic identifying number of the
      atom, e.g., the P0Atom is category 0 and the P3Atom is category
      3.
     */
    // enum { ATOM_CATEGORY = 99 };

    /**
      BITS_PER_ATOM is the total number of bits that each atom of this
      configuration will occupy.
     */
    // enum { BITS_PER_ATOM = 64 };

    /**
      ATOM_TYPE_BITS is the total number of type bits that each atom of this
      configuration can represent.
     */
    // enum { ATOM_TYPE_BITS = 16 };

    /**
      ATOM_EMPTY_TYPE is the type number reserved to mean 'empty' in an atom of this
      configuration.
     */
    // enum { ATOM_EMPTY_TYPE = 0xffff };

    /**
       ATOM_UNDEFINED_TYPE is the type number reserved to mean
       'undefined/illegal/subatomic' in an atom of this configuration.
    */
    // enum { ATOM_UNDEFINED_TYPE = 0x0000 };

    /**
      ATOM_FIRST_STATE_BIT is the bit number of the first (lowest)
      user-accessible state bit in the atom of this configuration.
     */
    // enum { ATOM_FIRST_STATE_BIT = 25 };

    /**
      ATOM_LAST_STATE_BIT is the bit number of the last (highest)
      user-accessible state bit in the atom of this configuration.
     */
    // enum { ATOM_LAST_STATE_BIT = BITS_PER_ATOM - 1 };
  };

} /* namespace MFM */

#endif /*ATOMCONFIG_H*/
