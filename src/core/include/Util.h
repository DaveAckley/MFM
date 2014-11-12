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
   * Generate a u64 constant that works on i386 and amd64, without
   * using the 'ULL' suffix that -ansi rejects, from two 32 bit pieces
   * (likely, though not actually necessarily, expressed as hex
   * constants).
   */
  inline u64 HexU64(const u32 hi, const u32 lo) {
    return ((((u64) hi)<<32)|lo);
  }

  /**
   * Generate a u64 constant that works on i386 and amd64, without
   * using the 'ULL' suffix that -ansi rejects, from two pieces
   * representing the bottom nine decimal digits in 'ones', and
   * the remaining upper decimal digits in 'billions'.  For example:
   * DecU64(123456,789101112) == 123456789101112ULL
   */
  inline u64 DecU64(const u32 billions, const u32 ones) {
    return ((((u64) billions)*1000000000) + ones);
  }

  inline s32 _SignExtend32(u32 val, u32 bitwidth) {
    return ((s32)(val<<(32-bitwidth)))>>(32-bitwidth);
  }

  inline s64 _SignExtend64(u64 val, u32 bitwidth) {
    return ((s64)(val<<(64-bitwidth)))>>(64-bitwidth);
  }

  inline u32 _GetNOnes32(u32 bitwidth) {
    return (bitwidth >= 32) ? (u32) -1 : (((u32)1)<<bitwidth)-1;
  }

  inline u64 _GetNOnes64(u32 bitwidth) {
    return (bitwidth >= 64) ? HexU64((u32)-1,(u32)-1) : (((u64)1)<<bitwidth)-1;
  }

  inline u32 _ShiftToBitNumber32(u32 value, u32 bitpos) {
    return value<<bitpos;
  }

  inline u64 _ShiftToBitNumber64(u32 value, u32 bitpos) {
    return ((u64) value)<<bitpos;
  }

  inline u32 _ShiftFromBitNumber32(u32 value, u32 bitpos) {
    return value>>bitpos;
  }

  inline u64 _ShiftFromBitNumber64(u64 value, u32 bitpos) {
    return value>>bitpos;
  }

  inline u32 _GetMask32(u32 bitpos, u32 bitwidth) {
    return _ShiftToBitNumber32(_GetNOnes32(bitwidth),bitpos);
  }

  inline u64 _GetMask64(u32 bitpos, u32 bitwidth) {
    return _ShiftToBitNumber64(_GetNOnes64(bitwidth),bitpos);
  }

  inline u32  _ExtractField32(u32 val, u32 bitpos,u32 bitwidth) {
    return _ShiftFromBitNumber32(val,bitpos)&_GetNOnes32(bitwidth);
  }

  inline u32  _ExtractUint32(u32 val, u32 bitpos,u32 bitwidth) {
    return _ExtractField32(val,bitpos,bitwidth);
  }

  inline s32  _ExtractSint32(u32 val, u32 bitpos,u32 bitwidth) {
    return _SignExtend32(_ExtractField32(val,bitpos,bitwidth),bitwidth);
  }

  inline u32 _getParity32(u32 v) {
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
  }

  // v must be <= 0x7fffffff
  inline u32 _getNextPowerOf2(u32 v) {
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    return v+1;
  }

  /**
   * Right-aligned mask generation.  Returns 0xFFFFFFFF if \a length
   * >= 32
   */
  inline u32 MakeMaskClip(const u32 length) {
    if (length<32) return (1u << length) - 1;
    return -1;
  }

  inline u32 PopCount(const u32 bits) {
    return __builtin_popcount(bits); // GCC
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
  inline T SGN(T x) {
    return (T) ((x > 0) ? 1 : (x < 0 ? -1 : 0));
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

  /** Template metaprogramming support for size-dependent integral types */
  template<u32 BITS> struct PickIType_;

  /** Template metaprogramming support for an 8 bit integral type */
  template<> struct PickIType_<8> { typedef u8 type; };

  /** Template metaprogramming support for a 16 bit integral type */
  template<> struct PickIType_<16> { typedef u16 type; };

  /** Template metaprogramming support for a 32 bit integral type */
  template<> struct PickIType_<32> { typedef u32 type; };

  /** Template metaprogramming support for a 64 bit integral type */
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

  /**
   * Counts the number of digits in a given number, in a given base.
   *
   * @param num The number to count the digits of.
   *
   * @param base The base to interpret \c num in when counting digits.
   *
   * @returns The digits of \c num when interpreted in \c base base.
   */
  extern u32 DigitCount(u32 num, u32 base);

  /**
   * Encodes an integral number as a series of alphabetic bytes.
   *
   * @param num The number to encode
   *
   * @param output The char buffer to output the encoded number to,
   * which must be at least 8 bytes long
   *
   * @return the first up-to-8 bytes that output points at are
   * modified to contain a null-terminated string of alphabetic
   * characters.
   */
  extern void IntAlphaEncode(u32 num, char* output);

  /**
   * Pauses the calling thread for a specified amount of time, using
   * (nominally) nanosecond precision.  (The actual granularity is
   * determined by the operating system and is usually in the
   * hundreds of nanos at least.)  The calling thread sleeps for the
   * number of seconds plus the number of nanoseconds specified.
   *
   * @param seconds The number of seconds that the calling thread
   *                should sleep for.
   *
   * @param nanos The number of nanoseconds that the calling thread
   *              should sleep for.
   */
  extern void Sleep(u32 seconds, u64 nanos) ;

  /**
   * Pauses the calling thread for more or less a specified number of
   * milliseconds.
   *
   * @param milliseconds The number of milliseconds that the calling thread
   *                should sleep for.
   */
  inline void SleepMsec(u32 milliseconds) {
    const u32 K = 1000;
    const u32 M = K*K;
    Sleep(milliseconds/K, (milliseconds%K)*DecU64(0,M));
  }

  /**
   * Pauses the calling thread for more or less a specified number of
   * microseconds.
   *
   * @param microseconds The number of microseconds that the calling
   *                thread should sleep for.
   */
  inline void SleepUsec(u32 microseconds) {
    const u32 K = 1000;
    const u32 M = K*K;
    Sleep(microseconds/M, (microseconds%M)*DecU64(0,K));
  }
}

#endif /* UTIL_H */
