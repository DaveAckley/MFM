#ifndef BITVECTOR_H      /* -*- C++ -*- */
#define BITVECTOR_H

#include "itype.h"
#include <stdio.h>  /* for FILE */
#include <climits>  /* for CHAR_BIT */
#include <stdlib.h> /* for abort */

namespace MFM {

  inline s32 _SignExtend32(u32 val, u32 bitwidth) {
    return ((s32)(val<<(32-bitwidth)))>>bitwidth;
  }

  inline s64 _SignExtend64(u64 val, u32 bitwidth) {
    return ((s64)(val<<(64-bitwidth)))>>bitwidth;
  }

  inline u32 _GetNOnes32(u32 bitwidth) {
    return (((u32)1)<<bitwidth)-1;
  }

  inline u64 _GetNOnes64(u32 bitwidth) {
    return (((u64)1)<<bitwidth)-1;
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

    u32 GetLength() const {
      return BITS;
    }

    typedef u32 BitUnitType;
    static const u32 BITS_PER_UNIT = sizeof(BitUnitType) * CHAR_BIT;

    static const u32 ARRAY_LENGTH = (BITS + BITS_PER_UNIT - 1) / BITS_PER_UNIT;
  private:
    BitUnitType m_bits[ARRAY_LENGTH];

    /**
     * Low-level mask generation.  No checking is done: Caller g'tees
     * length <= 32
     */
    inline static u32 MakeMask(const u32 length) {
      if (length<32) return (1u << length) - 1;
      return -1;
    }

    /**
     * Low-level raw bitvector writing to a single array element.
     * startIdx==0 means the leftmost bit (MSB).  No checking is done:
     * Caller guarantees (1) idx is valid and (2) startIdx + length <=
     * 32
     */
    inline void WriteToUnit(const u32 idx, const u32 startIdx, const u32 length, const u32 value) {
      if (length==0) return;
      const u32 shift = BITS_PER_UNIT - (startIdx + length);
      u32 mask = MakeMask(length) << shift;
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
      return (m_bits[idx] >> shift) & MakeMask(length);
    }

    void WriteBit(int idx, bool bit);

    bool ReadBit(int idx);

    bool ToggleBit(int idx);

  public:

    BitVector();

    BitVector(const BitVector & other);

    BitVector(const u32 * const values);

    inline u32 Read(const u32 startIdx, const u32 length) const;

    void Write(const u32 startIdx, const u32 length, const u32 value);

    void Clear();

    void Print(FILE* ostream) const;
			   
  };
} /* namespace MFM */

#include "BitVector.tcc"

#endif /*BITVECTOR_H*/
