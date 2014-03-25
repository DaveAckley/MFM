#ifndef BITFIELD_H      /* -*- C++ -*- */
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
    
    static u32 Read(const BV & bv) {
      return bv.Read(START,LENGTH);
    }

    static void Write(BV & bv, u32 val) {
      bv.Write(START,LENGTH,val);
    }
  };
} /* namespace MFM */

#include "BitField.tcc"

#endif /*BITFIELD_H*/

