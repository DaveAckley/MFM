/* -*- C++ -*- */
#include "Fail.h"
#include "BitVector.h"

namespace MFM {

  template <class EC>
  BitRef<EC>::BitRef(u32 p, u32 l) {
    const u32 BITS_PER_UNIT = BitVector<1>::BITS_PER_UNIT;
    MFM_API_ASSERT_ARG(p + l <= BITS_PER_ATOM);
    const u32 startIdx = p;
    const u32 length = l;
    const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
    const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
    const bool hasSecondUnit = (firstUnitFirstBit + length) > BITS_PER_UNIT;
    const u32 firstUnitLength = hasSecondUnit ? BITS_PER_UNIT - firstUnitFirstBit : length;
    const u32 firstUnitShift = BITS_PER_UNIT - (firstUnitFirstBit + firstUnitLength);

    pos = p;
    len = l;

    idx = firstUnitIdx;
    len1 = firstUnitLength;
    shift1 = firstUnitShift;
    mask1 = MakeMaskClip(firstUnitLength) << firstUnitShift;

    if (hasSecondUnit) {
      len2 = length - firstUnitLength;
      if (len2 > 32) {
        // If len2 >= 32, then we need the entirety of the second unit
        // -- so we don't need a shift or a mask for that!  Instead,
        // we'll set up for len2-32, which is the amount by which we
        // will be stretching into a third unit, where we do need a
        // shift and mask.
        len2 -= 32;
      }
      shift2 = BITS_PER_UNIT - len2;
      mask2 = MakeMaskClip(length - firstUnitLength) << (BITS_PER_UNIT - (length - firstUnitLength));
    } else {
      shift2 = len2 = 0;
      mask2 = 0;
    }
  }

  template <class EC>
  u32 BitRef<EC>::Read(const T & stg) const 
  { 
    const BV & bv = stg.GetBits();
    u32 val = (bv.m_bits[idx] & mask1) >> shift1;
    if (len2)
      val = (val<<len2) | ((bv.m_bits[idx+1] & mask2) >> shift2);
    return val;
  }

  template <class EC>
  void BitRef<EC>::Write(T & stg, u32 val) const 
  { 
    BV & bv = stg.GetBits();
    bv.m_bits[idx] =
      (bv.m_bits[idx] & ~mask1) | (((val >> len2) << shift1) & mask1);
    if (len2) 
      bv.m_bits[idx+1] = 
        (bv.m_bits[idx+1] & ~mask2) | (val << shift2); // can just shift out the idx bits
  }

  template <class EC>
  u64 BitRef<EC>::ReadLong(const T & stg) const 
  { 
    const BV & bv = stg.GetBits();
    u64 val = (bv.m_bits[idx] & mask1) >> shift1;
    if (len2) {
      u32 last = idx+1;
      if (len1 + len2 < len)            // detect third unit use
        val = (val<<32) | bv.m_bits[last++]; // OK, we need all of unit 2
      val = (val<<len2) | ((bv.m_bits[last] & mask2) >> shift2);
    }
    return val;
  }

  template <class EC>
  void BitRef<EC>::WriteLong(T & stg, u64 val) const 
  { 
    BV & bv = stg.GetBits();
    bv.m_bits[idx] =
      (bv.m_bits[idx] & ~mask1) | (u32) (((val >> (len - len1)) << shift1) & mask1);
    if (len2) {
      u32 last = idx+1;

      if (len1 + len2 < len) 
        bv.m_bits[last++] =  (u32) (val >> (len - len1 - 32)); // need all of unit2

      bv.m_bits[last] = 
        (bv.m_bits[last] & ~mask2) | (u32) (val << shift2); // can just shift out the idx bits
    }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(u32 pos, u32 len, T& stg, const UlamClass<EC> * effself) 
    : m_effSelf(effself) 
    , m_stg(stg)
    , m_ref(pos + POS_ORIGIN, len)
  { 
    MFM_API_ASSERT_ARG(pos + POS_ORIGIN + len <= T::BITS);
  }

  template <class EC>
  UlamRef<EC>::UlamRef(UlamRef & existing, u32 pos, u32 len, const UlamClass<EC> * effself) 
    : m_effSelf(effself)
    , m_stg(existing.m_stg)
    , m_ref(pos + existing.GetPos() + POS_ORIGIN, len)
  {
    MFM_API_ASSERT_ARG(pos + len <= existing.GetLen());
  }


} //MFM
