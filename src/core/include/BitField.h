/* -*- C++ -*- */
#ifndef BITFIELD_H
#define BITFIELD_H

#include "BitVector.h"

namespace MFM {

  /**
   * A field of up to 32 contiguous bits of a BitVector
   */
  template <class BV, u32 L, u32 I>
  class BitField
  {
  public:
    enum {
      BITS = BV::BITS,
      START = I,
      LENGTH = L,
      END = L+I
    };
    enum { BITFIELD_ERROR_OVERFLOWED_BITS = (END+1)/((BITS+1)/(END+1)) };
    enum { BITFIELD_ERROR_FIELD_TOO_LONG = (LENGTH+1)/((32+1)/(LENGTH+1)) };

    /**
     * Reads the contents of a window (which is represented by this
     * BitField) from a specified BV into a u32.
     *
     * @param bv The BV to read bits from.
     *
     * @returns the bits inside the window specified by this BitField
     *          which reside in bv.
     */
    static u32 Read(const BV & bv) {
      return bv.Read(START,LENGTH);
    }

    /**
     * writes the contents of a u32 into a window (which is represented by this
     * BitField) of a specified BV.
     *
     * @param bv The BV to write bits to.
     */
    static void Write(BV & bv, u32 val) {
      bv.Write(START,LENGTH,val);
    }
  };
} /* namespace MFM */

#include "BitField.tcc"

#endif /*BITFIELD_H*/
