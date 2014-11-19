/*                                              -*- mode:C++ -*-
  BitVector.h Extended integral type
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
  \file BitVector.h Extended integral type
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef BITVECTOR_H
#define BITVECTOR_H

#include "itype.h"
#include "ByteSink.h"
#include "ByteSource.h"
#include "Util.h"   /* for MakeMaskClip */
#include <climits>  /* for CHAR_BIT */
#include <stdlib.h> /* for abort */

namespace MFM {

  /**
   * A bit vector with reasonably fast operations
   *
   * BITS should be a multiple of BITS_PER_UNIT (currently
   * 32). Otherwise there will, at least, be some wasted space -- and
   * may be other issues.
   */
  template <u32 B>
  class BitVector
  {
  public:
    enum { BITS = B };

    /**
     * Gets the number of bits that may be indexed inside this
     * BitVector.
     *
     * @returns The number of bits that may be indexed inside this
     *          BitVector.
     *
     * @remarks The actual number of bits used in this BitVector is
     * a multiple of \c sizeof(BitUnitType) .
     */
    u32 GetLength() const
    {
      return BITS;
    }

    typedef u32 BitUnitType;
    static const u32 BITS_PER_UNIT = sizeof(BitUnitType) * CHAR_BIT;

    static const u32 ARRAY_LENGTH = (BITS + BITS_PER_UNIT - 1) / BITS_PER_UNIT;

  private:
    BitUnitType m_bits[ARRAY_LENGTH];

    /**
     * Low-level raw bitvector writing to a single array element.
     * startIdx==0 means the leftmost bit (MSB).  No checking is done:
     * Caller guarantees (1) idx is valid and (2) startIdx + length <=
     * 32
     */
    inline void WriteToUnit(const u32 idx, const u32 startIdx, const u32 length, const u32 value) {
      if (length == 0) return;
      const u32 shift = BITS_PER_UNIT - (startIdx + length);
      u32 mask = MakeMaskClip(length) << shift;
      m_bits[idx] = (m_bits[idx] & ~mask) | ((value << shift) & mask);
    }

    /**
     * Low-level raw bitvector reading from a single array element.
     * startIdx==0 means the leftmost bit (MSB).  No checking is done:
     * Caller guarantees (1) idx is valid and (2) startIdx + length <=
     * 32
     */
    inline u32 ReadFromUnit(const u32 idx, const u32 startIdx, const u32 length) const {
      if (length==0) { return 0; }
      if(idx >= ARRAY_LENGTH) abort();
      const u32 shift = BITS_PER_UNIT - (startIdx + length);
      return (m_bits[idx] >> shift) & MakeMaskClip(length);
    }

  public:

    /**
     * Writes a specified value to a particular bit in this BitVector.
     *
     * @param idx The bit to set, where the MSB is index 0.
     *
     * @param bit The value to set the bit at \c idx to.
     */
    void WriteBit(u32 idx, bool bit);

    /**
     * Reads a specified value from a particular bit in this BitVector.
     *
     * @param idx The bit to read, where the MSB is index 0.
     *
     * @returns The value of the bit at \c idx index.
     */
    bool ReadBit(u32 idx);

    /**
     * Constructs a new BitVector. Set parameters of this BitVector
     * using the template arguments. All bits are initialized to \c 0 .
     */
    BitVector();

    /**
     * Copy-constructor for a BitVector. Creates an identical copy of
     * the specified BitVector.
     *
     * @param other The BitVector to copy properties of.
     */
    BitVector(const BitVector & other);

    /**
     * Constructs a BitVector with specified inital value.
     *
     * @param values A pointer to a big-endian array of values to
     *               initialize this BitVector to. This array must
     *               contain at least as many bits as this BitVector
     *               can hold (specified as a template parameter).
     */
    BitVector(const u32 * const values);

    /**
     * Reads up to 32 bits of a particular section of this BitVector.
     *
     * @param startIdx The index of the first bit to read inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to read from this
     *               BitVector. This should be in the range \c [1,32] .
     *
     * @returns The bits read from the particular section of this
     *          BitVector.
     */
    inline u32 Read(const u32 startIdx, const u32 length) const;

    /**
     * Writes up to 32 bits of a specified u32 to a section of this BitVector.
     *
     * @param startIdx The index of the first bit to write inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to write to this
     *               BitVector. This should be in the range \c [1,32] .
     *
     * @param value The bits to write to the specified section of this
     *              BitVector.
     */
    void Write(const u32 startIdx, const u32 length, const u32 value);

    /**
     * Reads up to 64 bits of a particular section of this BitVector.
     *
     * @param startIdx The index of the first bit to read inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to read from this
     *               BitVector. This should be in the range \c [0,64] .
     *
     * @returns The bits read from the particular section of this
     *          BitVector.
     */
    inline u64 ReadLong(const u32 startIdx, const u32 length) const;

    /**
     * Writes up to 64 bits of a specified u64 to a section of this BitVector.
     *
     * @param startIdx The index of the first bit to write inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to write to this
     *               BitVector. This should be in the range \c [0,64] .
     *
     * @param value The bits to write to the specified section of this
     *              BitVector.
     */
    void WriteLong(const u32 startIdx, const u32 length, const u64 value);

    /**
     * Sets the bit at a specified index in this BitVector.
     *
     * @param idx The index of the bit to set, where the MSB is
     *        indexed at \c 0 .
     */
    void SetBit(const u32 idx) {
      Write(idx, 1, 1);
    }

    /**
     * Clears the bit at a specified index in this BitVector.
     *
     * @param idx The index of the bit to clear, where the MSB is
     *        indexed at \c 0 .
     */
    void ClearBit(const u32 idx) {
      Write(idx, 1, 0);
    }

    /**
     * Reads a single bit at a specified index from this BitVector.
     *
     * @param idx The index of the bit to read.
     *
     * @returns \c 1 if this bit is set, else \c 0 .
     */
    bool ReadBit(const u32 idx) const {
      return Read(idx, 1) != 0;
    }

    /**
     * Flips the bit at a specified index in this BitVector.
     * (i.e. bv[idx] = !bv[idx])
     *
     * @param idx The index of the bit to toggle, where the MSB is
     *        indexed at \c 0 .
     */
    bool ToggleBit(const u32 idx);

    /**
     * Set a contiguous range of bits, so they all have value 1.
     *
     * @param startIdx The index of the first bit to set inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to set in this BitVector.  If
     *               this length would exceed the number of bits in
     *               the BitVector, the excess length is ignored.
     *
     */
    void SetBits(const u32 startIdx, const u32 length) {
      StoreBits(0xffffffff, startIdx, length);
    }

    /**
     * Clear a contiguous range of bits, so they all have value 0.
     *
     * @param startIdx The index of the first bit to clear inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to clear in this BitVector.
     *               If this length would exceed the number of bits in
     *               the BitVector, the excess length is ignored.
     *
     */
    void ClearBits(const u32 startIdx, const u32 length)  {
      StoreBits(0, startIdx, length);
    }

    /**
     * Store a bit value (or pattern) into a contiguous range of bits,
     * so they all have that value.
     *
     * @param bits The bits to store.  The supplied bits are tiled
     *                 onto the contiguous range of bits as many times
     *                 as needed to cover the given length, with the
     *                 u32 bits aligned to the underlying u32 storage
     *                 units. To avoid confusion, pass only 0x0 or
     *                 0xffffffff as bits.
     *
     * @param startIdx The index of the first bit to store inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to store in this BitVector.
     *               If this length would exceed the number of bits in
     *               the BitVector, the excess length is ignored.
     *
     */
    void StoreBits(const u32 bits, const u32 startIdx, const u32 length) ;

    /**
     * Sets all bits of this BitVector to \c 0 .
     */
    void Clear();

    /**
     * Prints the bits held in this BitVector to a specified ByteSink in
     * hex format.
     *
     * @param ostream The ByteSink to print bits to.
     */
    void Print(ByteSink & ostream) const;

    /**
     * Prints the bits held in this BitVector to a specified ByteSink
     * in base 2 format.
     *
     * @param ostream The ByteSink to print bits to.
     */
    void PrintBase2(ByteSink& ostream) const;

    /**
     * Prints the bytes held in this BitVector to a specified ByteSink
     * as unformatted raw bytes
     *
     * @param ostream The ByteSink to print bytes to.
     */
    void PrintBytes(ByteSink& ostream) const;

    /**
     * Load this BitVector from a specified ByteSource by reading BITS
     * / 4 bytes in hex format.  Skips leading whitespace.  Returns \c
     * true if all went well; returns false if insufficient
     * consecutive hex bytes were available to fully initialize the
     * BitVector .  When false is returned the ByteSource will be
     * positioned just before the first unacceptable byte encountered,
     * and the BitVector will be unchanged.
     *
     * @param istream The ByteSource to read bytes from.
     *
     * @returns \c true if all went well, else \c false .
     */
    bool Read(ByteSource & bs);

    /**
     * Load this BitVector from a specified ByteSource by reading BITS
     * bits in base 2 format. Skips leading whitespace. Returns \c
     * true if all went well; returns \c false if insufficient
     * consecutive '0's and '1's bytes were available to fully
     * initialize the BitVector . When \c false is returned, the
     * ByteSource will be positioned just before the first
     * unacceptable byte encountered, and the BitVector will be
     * unchanged.
     *
     * @param istream The ByteSource to read bytes from.
     *
     * @returns \c true if all went well, else \c false .
     */
    bool ReadBase2(ByteSource& bs);

    /**
     * Load this BitVector from a specified ByteSource by reading raw
     * bytes of the form written by PrintBytes.  Returns \c true if
     * all went well; returns \c false if insufficient bytes were
     * available to fully initialize the BitVector . When \c false is
     * returned, the ByteSource position is undefined, but the
     * BitVector is unchanged.
     *
     * @param istream The ByteSource to read bytes from.
     *
     * @returns \c true if all went well, else \c false .
     */
    bool ReadBytes(ByteSource& bs);

    bool operator==(const BitVector & rhs) const;

  };
} /* namespace MFM */

#include "BitVector.tcc"

#endif /*BITVECTOR_H*/
