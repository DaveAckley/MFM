#ifndef BITVECTOR_H      /* -*- C++ -*- */
#define BITVECTOR_H

#include "itype.h"
#include <stdio.h>  /* for FILE */
#include <climits>  /* for CHAR_BIT */

namespace MFM {

  /** 
   * A bit vector with reasonably fast operations
   *
   * BITS should be a multiple of BITS_PER_UNIT (currently
   * 32). Otherwise there will, at least, be some wasted space -- and
   * may be other issues.
   */

  template <u32 BITS>
  class BitVector
  {
  public:
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
      if(!length) { return length; }
      if(idx >= 2) { printf("Whoa! %d %d %d\n", idx, startIdx, length); return 0; }
      const u32 shift = BITS_PER_UNIT - (startIdx + length);
      return (m_bits[idx] >> shift) & MakeMask(length);
    }

    void WriteBit(int idx, bool bit);

    bool ReadBit(int idx);
  public:

    BitVector();

    BitVector(const BitVector & other);

    BitVector(const u32 * const values);

    u32 Read(const u32 startIdx, const u32 length) const;

    void Write(const u32 startIdx, const u32 length, const u32 value);

#if 0 // Considered harmful..
    /* 
     * Inserts a value at startIdx. Pushes the values
     * already there down length bits.
     */
    void Insert(u32 startIdx, u32 length, u32 value);
#endif

#if 0
    /*
     * Removes length bits from this bitvector, then pushes
     * all bits to the right of them back length bits.
     */
    void Remove(u32 startIdx, u32 length);
#endif

    void Print(FILE* ostream) const;
			   
  };
} /* namespace MFM */

#include "BitVector.tcc"

#endif /*BITVECTOR_H*/
