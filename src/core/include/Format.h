/* -*- mode:C++ -*- */

/*
  Format.h Type codes for formatted IO in various bases, etc
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
   \file Format.h Type codes for formatted IO in various bases, etc
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef BASECODE_H
#define BASECODE_H

namespace MFM {

  /**
     Formatting codes for use with \ref Print(u32, Format::Type), \ref
     Print(s32, Format::Type), and \ref Print(u64, Format::Type).
  */
  struct Format {
    typedef enum {
      DEC = 10,      /**< Format in decimal, signed if negative (in most contexts) */
      HEX = 16,      /**< Format in hexadecimal (base 16, using 0-9A-Z), unsigned */
      OCT = 8,       /**< Format in octal (base 8, using 0-7), unsigned */
      BIN = 2,       /**< Format in binary (base 2, using 0-1), unsigned */
      BYTE = 0,      /**< Format a single byte directly, as an unaltered 8 bit value */

      BEU16 = -1,    /**< Format a u16 (unsigned 2 bytes) in big endian (network order) */
      BEU32 = -2,    /**< Format a u32 (unsigned 4 bytes) in big endian (network order) */
      BEU64 = -3,    /**< Format a u64 (unsigned 8 bytes) in big endian (network order) */
      LEX32 = -4,    /**< Format a u32 (unsigned 4 bytes) in leximited decimal */
      LXX32 = -5,    /**< Format a u32 (unsigned 4 bytes) in leximited hexadecimal */
      LEX64 = -6,    /**< Format a u64 (unsigned 8 bytes) in leximited decimal */
      LXX64 = -7,    /**< Format a u64 (unsigned 8 bytes) in leximited hexadecimal */
      LEXHD = -8,    /**< Format a u32 (unsigned 4 bytes) as a leximited format header */
      B36 = 36       /**< Format in base 36 ("hexatrigintimal"?, using 0-9A-Z), unsigned */
    } Type;
  };
}

#endif /*BASECODE_H*/

