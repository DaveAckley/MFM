/*                                              -*- mode:C++ -*-
  BitVector.h Extended integral type
  Copyright (C) 2014, 2018, 2020 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2020-2021 The Living Computation Foundation. All rights reserved.

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
  \date (C) 2014,2018,2020-2021 All rights reserved.
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

  template <class EC> class BitRef; // FORWARD

  template <u32 B> class BitVector; // FORWARD

  typedef BitVector<96> BV96;

  typedef BitVector<8192> BV8K;

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
    enum { BITS = ((B==0) ? 1 : B) }; // Avoid zero length array

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
    template <typename EC> friend class BitRef;
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
    void WriteBit(u32 idx, bool bit)
    {
      MFM_API_ASSERT_ARG(idx < B);
      WriteBitUnsafe(idx, bit);
    }

    /**
     * Reads a single bit at a specified index from this BitVector.
     *
     * @param idx The index of the bit to read.
     *
     * @returns \c true if this bit is set, else \c false .
     */
    bool ReadBit(const u32 idx) const
    {
      MFM_API_ASSERT_ARG(idx < B);
      return ReadBitUnsafe(idx);
    }

    /**
     * Constructs a new BitVector. Set parameters of this BitVector
     * using the template arguments. All bits are initialized to \c 0 .
     */
    BitVector();

#if 1 // Mon Jan 11 20:33:04 2016 Need this for Ui_Ut copy ctors?
//#if 0 // Fri Mar 13 16:04:59 2015 XXX TESTING GCC CODE GEN IMPACTS
    /**
     * Copy-constructor for a BitVector. Creates an identical copy of
     * the specified BitVector.
     *
     * @param other The BitVector to copy properties of.
     */
    BitVector(const BitVector & other);

    BitVector& operator=(const BitVector & rhs); //explicit for c++11

#endif // Fri Mar 13 16:04:59 2015 XXX TESTING GCC CODE GEN IMPACTS

    /**
     * Constructs a BitVector with specified initial value.
     *
     * @param values A pointer to an array of 'appropriate-endian'
     *               data -- meaning the byte order of the data must
     *               match that of the machine running this code -- to
     *               initialize this BitVector.  (For example,
     *               executing (values[0]>>31) on this machine must
     *               obtain the value to be returned by GetBit(0)).
     *               The values array must contain at least as many
     *               bits as this BitVector can hold (specified as a
     *               template parameter).
     */
    BitVector(const u32 * const values);

    /**
     * Constructs a BitVector that's all zeroes except the rightmost
     * 32 bits have a specified initial value.
     *
     * @param value A value to place in the rightmost 32 bits of the
     *               BitVector.
     */
    BitVector(const u32 value);

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
     *          BitVector, right-justified.
     */
    inline u32 Read(const u32 startIdx, const u32 length) const
    {
      if (length == 0)
	return 0;

      MFM_API_ASSERT_ARG(startIdx + length <= B);
      MFM_API_ASSERT_ARG(length <= sizeof(BitUnitType) * CHAR_BIT);

      /* See Write(u32,u32,u32) for theory, such as it is */

      const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
      const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
      const bool hasSecondUnit = (firstUnitFirstBit + length) > BITS_PER_UNIT;
      const u32 firstUnitLength = hasSecondUnit ? BITS_PER_UNIT-firstUnitFirstBit : length;

      u32 ret = ReadFromUnit(firstUnitIdx, firstUnitFirstBit, firstUnitLength);

      // NOTE: The ARRAY_LENGTH > 1 clause of the following 'if' is
      // strictly unnecessary, since it is implied by hasSecondUnit --
      // but without it, gcc's optimizer (at least in some versions)
      // mistakenly declares an array bounds warning (which we treat as
      // an error) when inlining the code involving firstUnitIdx + 1
      if (ARRAY_LENGTH > 1 && hasSecondUnit) {
	const u32 secondUnitLength = length - firstUnitLength;
	ret = (ret << secondUnitLength) | ReadFromUnit(firstUnitIdx + 1, 0, secondUnitLength);
      }
      return ret;
    } //Read

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
    inline void Write(const u32 startIdx, const u32 length, const u32 value)
    {

      if (length == 0) return;

      MFM_API_ASSERT_ARG(startIdx + length <= B);
      MFM_API_ASSERT_ARG(length <= sizeof(BitUnitType) * CHAR_BIT);

      /* Since we're writing no more than 32 bits into an array of 32 bit
	 words, we can't need to touch more than two of them.  So unroll
	 the loop.
      */

      const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
      const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
      const bool hasSecondUnit = (firstUnitFirstBit + length) > BITS_PER_UNIT;
      const u32 firstUnitLength = hasSecondUnit ? BITS_PER_UNIT - firstUnitFirstBit : length;

      WriteToUnit(firstUnitIdx, firstUnitFirstBit, firstUnitLength, value >> (length - firstUnitLength));

      // NOTE: The ARRAY_LENGTH > 1 clause of the following 'if' is
      // strictly unnecessary, since it is implied by hasSecondUnit --
      // but without it, gcc's optimizer (at least in some versions)
      // mistakenly declares an array bounds warning (which we treat as
      // an error) when inlining the code involving firstUnitIdx + 1
      if (ARRAY_LENGTH > 1 && hasSecondUnit)
	WriteToUnit(firstUnitIdx + 1, 0, length - firstUnitLength, value);
    } //Write

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
     *          BitVector, right-justified.
     */
    inline u64 ReadLong(const u32 startIdx, const u32 length) const
    {
      const u32 firstLen = MIN((const u32) 32,length);
      const u32 secondLen = length - firstLen;
      u64 ret = Read(startIdx + secondLen, firstLen);
      if (secondLen > 0)
	{
	  ret |= ((u64) Read(startIdx, secondLen)) << firstLen;
	}
      return ret;
    }

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
    // void WriteLong(const u32 startIdx, const u32 length, const u64 value);
    inline void WriteLong(const u32 startIdx, const u32 length, const u64 value)
    {
      const u32 firstLen = MIN((const u32) 32,length);
      const u32 secondLen = length - firstLen;
      Write(startIdx + secondLen, firstLen, (u32) value);
      if (secondLen > 0)
	{
	  Write(startIdx, secondLen, ((u32) (value >> firstLen)));
	}
    }

    /**
     * Copy an arbitrary subsection of this BitVector to a different
     * BitVector \c dstbv.  Template parameter \c BITS is the size \c
     * dstbv.
     *
     * @param srcStartIdx The index of the first bit to read inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param dstStartIdx The index of the first bit to write inside
     *                 \c dstbv, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to copy.
     *
     * @param dstbv The destination BitVector<DBITS> that is written
     *              in positions [dstStartIdx, dstStartIdx + length - 1].
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this, or
     *                         srcStartIdx + length is greater than
     *                         the number of bits in this, or
     *                         dstStartIdx + length is greater than
     *                         the number of bits in dstbv
     *
     * @sa ReadBV, WriteBV
     *
     */
    template <u32 DBITS>
    inline void CopyBV(const u32 srcStartIdx, const u32 dstStartIdx, const u32 length, BitVector<DBITS> & dstbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &dstbv)); // Ensure distinct ptrs; can't move within yourself
      u32 amt = BITS_PER_UNIT;
      for (u32 i = 0; i < length; i += amt)
	{
	  if (i + amt > length) amt = length - i;
	  dstbv.Write(dstStartIdx + i, amt, this->Read(srcStartIdx + i, amt));
	}
    }

    /**
     * Reads an arbitrary subsection of this BitVector into all of \c
     * rtnbv.  Template parameter \c DBITS determines the number of
     * bits read.
     *
     * @param startIdx The index of the first bit to read inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param rtnbv The BitVector<DBITS> modified to hold the read bits .
     *
     * @sa WriteBV, CopyBV
     *
     */
    template <u32 DBITS>
    inline void ReadBV(const u32 startIdx, BitVector<DBITS> & rtnbv) const
    {
      this->CopyBV(startIdx, 0, DBITS, rtnbv);
    }

    /**
     * Writes all of an arbitrary BitVector starting a specified
     * position in this BitVector.  Template parameter \c BITS
     * determines the number of bits written.
     *
     * @param startIdx The index of the first bit to write inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param val The bits in \c [0, DBITS - 1] of val are written to
     *              the specified section of this BitVector.
     *
     * @sa ReadBV, CopyBV
     *
     */
    template<u32 DBITS>
    void WriteBV(const u32 startIdx, const BitVector<DBITS>& val)
    {
      val.CopyBV(0, startIdx, DBITS, *this);
    }

    /**
     * Reads up to 96 bits of a particular section of this BitVector.
     *
     * @param startIdx The index of the first bit to read inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to read from this
     *               BitVector. This should be in the range \c [0,96] .
     *
     * @returns The bits read from the particular section of this
     *          BitVector, left-justified in the BV96.
     */
    inline BV96 ReadBig(const u32 startIdx, const u32 length) const
    {
      BV96 val;
      this->CopyBV(startIdx, 0, length, val);
      return val;
    }

    /**
     * Writes up to 96 bits of a specified BV96 to a section of this BitVector.
     *
     * @param startIdx The index of the first bit to write inside this
     *                 BitVector, where the MSB is indexed at \c 0 .
     *
     * @param length The number of bits to write to this
     *               BitVector. This should be in the range \c [0,96] .
     *
     * @param value The bits \c [0, length - 1] of value are written
     *              to the specified section of this BitVector.
     */
    void WriteBig(const u32 startIdx, const u32 length, const BV96 value)
    {
      value.CopyBV(0, startIdx, length, *this);
    }

    /**
     * Gets the bit at a specified index in this BitVector.  (Same as
     * \c ReadBit).
     *
     * @param idx The index of the bit to access, where the MSB is
     *        indexed at \c 0 .
     */
    bool GetBit(u32 idx) const
    {
      return ReadBit(idx);
    }

    /**
     * Sets the bit at a specified index in this BitVector.
     *
     * @param idx The index of the bit to set, where the MSB is
     *        indexed at \c 0 .
     */
    void SetBit(const u32 idx) {
      MFM_API_ASSERT_ARG(idx < B);
      WriteBitUnsafe(idx, true);
    }

    /**
     * Clears the bit at a specified index in this BitVector.
     *
     * @param idx The index of the bit to clear, where the MSB is
     *        indexed at \c 0 .
     */
    void ClearBit(const u32 idx) {
      MFM_API_ASSERT_ARG(idx < B);
      WriteBitUnsafe(idx, false);
    }

    /**
     * Stores the given bit at a specified index in this BitVector.
     *
     * @param idx The index to store the bit into, where the MSB is
     *        indexed at \c 0 .
     */
    void StoreBit(const u32 idx, bool bit) {
      MFM_API_ASSERT_ARG(idx < B);
      WriteBitUnsafe(idx, bit);
    }

    void WriteBitUnsafe(const u32 idx, const bool bit) ;

    bool ReadBitUnsafe(const u32 idx) const ;

    /**
     * Flips the bit at a specified index in this BitVector.
     * (i.e. bv[idx] = !bv[idx])
     *
     * @param idx The index of the bit to toggle, where the MSB is
     *        indexed at \c 0 .
     */
    bool ToggleBit(const u32 idx)
    {
      MFM_API_ASSERT_ARG(idx < B);
      return ToggleBitUnsafe(idx);
    }

    bool ToggleBitUnsafe(const u32 idx) ;

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
     * Sets all bits of this BitVector to \c 1 .
     */
    void SetAllOnes();

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

    void ToArray(u32 array[ARRAY_LENGTH]) const;

    void FromArray(const u32 array[ARRAY_LENGTH]);

    /**
     * Compute the number of 1 (set) bits in this BitVector, starting
     * from index \c startIdx (default: 0) and counting the next \c
     * length bits (default: All of the bits from startIdx to the
     * end).
     *
     * @param startIdx First bit to include in the population count
     *
     * @param length The maximum number of bits to include in the
     * population count
     *
     * @returns The number of 1 bits counted.
     */
    u32 PopulationCount(const u32 startIdx = 0, const u32 length = B) const;

    /**
     * Shift LEFT an arbitrary number of bits of this BitVector to a different
     * BitVector \c dstbv.  Template parameter \c DBITS is the size \c
     * dstbv.
     *
     * @param shiftdist The number of bits to shift, where the MSB is indexed at \c 0 .
     *
     * @param dstbv The destination BitVector<DBITS> that is written
     *              in positions [0, length - shiftdist]. Cleared first.
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa CopyBV
     *
     */
    template <u32 DBITS>
    inline void _ShiftOpLeftBitsBV(const u32 shiftdist, BitVector<DBITS> & dstbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &dstbv)); // Ensure distinct ptrs; can't move within yourself
      dstbv.Clear();
      u32 length = this->GetLength();
      u32 startidx = (DBITS - shiftdist - (length - shiftdist));

      this->CopyBV(shiftdist, startidx, length - shiftdist, dstbv);
    }


    /**
     * Shift RIGHT by an arbitrary number of bits, this BitVector to a different
     * BitVector \c dstbv.  Template parameter \c DBITS is the size \c
     * dstbv.
     *
     * @param shiftdist The number of bits to shift, where the MSB is indexed at \c 0 .
     *
     * @param dstbv The destination BitVector<DBITS> that is written
     *              in positions [startidx, length - shiftdist]. Cleared first.
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa CopyBV
     *
     */
    template <u32 DBITS>
    inline void _ShiftOpRightBitsBV(const u32 shiftdist, BitVector<DBITS> & dstbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &dstbv)); // Ensure distinct ptrs; can't move within yourself
      dstbv.Clear();
      u32 length = this->GetLength();
      u32 startidx = (DBITS - (length - shiftdist));

      this->CopyBV(0u, startidx, (length - shiftdist), dstbv); //drops 'shiftdist' number of bits
    }


    /**
     * Bitwise OR this BitVector and another BitVector \c rbv, into a different
     * BitVector \c dstbv. All the same size, B.
     *
     * @param rbv The second bitvector, where the MSB is indexed at \c 0 .
     *
     * @param dstbv The destination BitVector<B> that is written
     *              in positions [0, B].
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa Read, Write
     *
     */
    inline void _BitwiseOrBitsBV(const BitVector<B> & rbv, BitVector<B> & dstbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &dstbv)); // Ensure distinct ptrs; can't move within yourself
      dstbv.Clear();

      u32 length = this->GetLength();;
      u32 amt = BITS_PER_UNIT;
      for (u32 i = 0; i < length; i += amt)
	{
	  if (i + amt > length) amt = length - i;
	  dstbv.Write(i, amt, this->Read(i, amt) | rbv.Read(i, amt));
	}
    }


    /**
     * Bitwise AND this BitVector and another BitVector \c rbv, into a different
     * BitVector \c dstbv. All the same size, B.
     *
     * @param rbv The second BitVector<B>, where the MSB is indexed at \c 0 .
     *
     * @param dstbv The destination BitVector<B> that is written
     *              in positions [0, B].
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa Read, Write
     *
     */
    inline void _BitwiseAndBitsBV(const BitVector<B> & rbv, BitVector<B> & dstbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &dstbv)); // Ensure distinct ptrs; can't move within yourself
      dstbv.Clear();

      u32 length = this->GetLength();
      u32 amt = BITS_PER_UNIT;
      for (u32 i = 0; i < length; i += amt)
	{
	  if (i + amt > length) amt = length - i;
	  dstbv.Write(i, amt, this->Read(i, amt) & rbv.Read(i, amt));
	}
    }


    /**
     * Bitwise XOR this BitVector and another BitVector \c rbv, into a different
     * BitVector \c dstbv. All the same size, B.
     *
     * @param rbv The second BitVector<B>, where the MSB is indexed at \c 0 .
     *
     * @param dstbv The destination BitVector<B> that is written
     *              in positions [0, B].
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa Read, Write
     *
     */
    inline void _BitwiseXorBitsBV(const BitVector<B> & rbv, BitVector<B> & dstbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &dstbv)); // Ensure distinct ptrs; can't move within yourself
      dstbv.Clear();

      u32 length = this->GetLength();
      u32 amt = BITS_PER_UNIT;
      for (u32 i = 0; i < length; i += amt)
	{
	  if (i + amt > length) amt = length - i;
	  dstbv.Write(i, amt, this->Read(i, amt) ^ rbv.Read(i, amt));
	}
    }

    /**
     * Bitwise COMPLEMENT (~) this BitVector into a different
     * BitVector \c dstbv. All the same size, B.
     *
     * @param dstbv The destination BitVector<B> that is written
     *              in positions [0, B].
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa _BitwiseXorBitsBV
     *
     */
    inline void _BitwiseComplementBitsBV(BitVector<B> & dstbv) const
    {
      BitVector<B> bvmask;
      bvmask.SetAllOnes();
      this->_BitwiseXorBitsBV(bvmask, dstbv);
    }


    /**
     * Bitwise Compare == this BitVector and another BitVector \c rbv, the same size, B.
     *
     * @param rbv The second BitVector<B>, where the MSB is indexed at \c 0 .
     *
     * returns true if equal equal, false if different
     *
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa Read
     *
     */
    inline bool _BinOpCompareEqEqBitsBV(const BitVector<B> & rbv) const
    {
      MFM_API_ASSERT_ARG(((void*) this) != ((void*) &rbv)); // Ensure distinct ptrs; can't move within yourself

      return *this == rbv; //overloaded op==
    }


    /**
     * Bitwise Compare NOT EQ this BitVector and another BitVector \c rbv, the same size, B.
     *
     * @param rbv The second BitVector<B>, where the MSB is indexed at \c 0 .
     *
     * returns true if equal equal, false if different
     *
     *
     * @fails ILLEGAL_ARGUMENT if dstbv is the same object as this
     *
     * @sa _BinOpCompareEqEqBitsBV
     *
     *
     */
    inline bool _BinOpCompareNotEqBitsBV(const BitVector<B> & rbv) const
    {
      return !(this->_BinOpCompareEqEqBitsBV(rbv));
    }


  };

} /* namespace MFM */

#include "BitVector.tcc"

#endif /*BITVECTOR_H*/
