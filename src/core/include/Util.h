/*                                              -*- mode:C++ -*-
  Util.h Globally acessible extension methods
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
  \file Util.h Globally acessible extension methods
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef UTIL_H
#define UTIL_H

#include "itype.h"
#include "math.h"
#include <string.h>

namespace MFM {

#define MARK_USED(X) ((void)(&(X)))

  template <const bool mustBeTrue>
  inline void COMPILATION_REQUIREMENT()
  {
    typedef char errorIfFalse[mustBeTrue == 0 ? -1 : 1];
    errorIfFalse t;
    MARK_USED(t);
  }

  /**
   * Right-aligned mask generation.  Returns 0xFFFFFFFF if \a length
   * >= 32
   */
  inline static u32 MakeMaskClip(const u32 length) {
    if (length<32) return (1u << length) - 1;
    return -1;
  }

  template <class T>
  inline T MAX(T x, T y) {
    return (x > y) ? x : y;
  }

  template <class T>
  inline T MIN(T x, T y) {
    return (x < y) ? x : y;
  }

  template <class T>
  inline T CLAMP(T min, T max, T val)
  {
    return val < min ? min : (val > max ? max : val);
  }

  template <class T>
  inline T ABS(T val)
  {
    return val > 0 ? val : (-val);
  }

  template <class T>
  inline double DISTANCE(T x1, T y1, T x2, T y2)
  {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  }

  template <u32 BITS>
  inline u32 PARITY_BITS(u32 value)
  {
    return PARITY_BITS<BITS/2>(value ^ (value>>(BITS/2)));
  }

  template <>
  inline u32 PARITY_BITS<1>(u32 value) {
    return value&1;
  }

  template <>
  inline u32 PARITY_BITS<2>(u32 value) {
    return (value^(value>>1))&1;
  }

  template <>
  inline u32 PARITY_BITS<3>(u32 value) {
    return (0x96 >> (value&0x3)) & 1;
  }

  template <>
  inline u32 PARITY_BITS<4>(u32 value)
  {
    // 0000  0
    // 0001  1
    // 0010  1
    // 0011  0
    // 0100  1
    // 0101  0
    // 0110  0
    // 0111  1
    // 1xxx  Same as above, just flipped:
    // 01101001 10010110
    // 0110 1001 1001 0110
    // 0x6996
    return (0x6996>>(value&0xf))&1;
  }

  template <class T>
  inline u32 PARITY(T value) {
    return PARITY_BITS<8*sizeof(T)>((u32) value);
  }

  template<u32 MIN_BITS>
  struct _ITypeSizes
  {
    enum { value =
           MIN_BITS <= 8  ?  8 :
           MIN_BITS <= 16 ? 16 :
           MIN_BITS <= 32 ? 32 :
                            64
    };
  };

  template<u64 NUMBER>
  struct _ITypeSizesForNumber
  {
    enum { value =
           NUMBER <= 0xff       ?  8 :
           NUMBER <= 0xffff     ? 16 :
           NUMBER <= 0xffffffff ? 32 :
                                  64
    };
  };

  template<u32 BITS> struct PickIType_;
  template<> struct PickIType_<8> { typedef u8 type; };
  template<> struct PickIType_<16> { typedef u16 type; };
  template<> struct PickIType_<32> { typedef u32 type; };
  template<> struct PickIType_<64> { typedef u64 type; };

  template<u32 BITS>
  struct UForBits : PickIType_<_ITypeSizes<BITS>::value> {};

  template<u64 NUMBER>
  struct UForNumber : PickIType_<_ITypeSizesForNumber<NUMBER>::value> {};

  /**
   * Counts the occurrences of a particular item in an array, using
   * the '==' operator to test for equality.
   *
   * @param array The array to search through
   *
   * @param item  The item to look for
   *
   * @returns The number of times that \c item occurs in \c array
   */
  template <class T>
  u32 ITEM_COUNT(T* array, T item, u32 arrSize)
  {
    u32 count = 0;
    u32 i;
    for(i = 0; i < arrSize; i++)
    {
      if(array[i] == item)
      {
	count++;
      }
    }
    return count;
  }
}

#endif /* UTIL_H */
