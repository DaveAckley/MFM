/* -*- C++ -*- */
#include "Fail.h"  /* For FAIL */
#include "Util.h"  /* For MAX */
#include <string.h> /* For memset, memcpy */

namespace MFM {

  template <u32 B>
  BitVector<B>::BitVector()
  {
    Clear();
  }

  template <u32 B>
  BitVector<B>::BitVector(const u32 * const values)
  {
    memcpy(m_bits,values,sizeof(m_bits));
  }

#if 1 // Mon Jan 11 20:33:04 2016 Need this for Ui_Ut copy ctors?
//#if 0 // Fri Mar 13 16:04:59 2015 XXX TESTING GCC CODE GEN IMPACTS
  template <u32 B>
  BitVector<B>::BitVector(const BitVector & other)
  {
    memcpy(m_bits,other.m_bits,sizeof(m_bits));
  }

  template <u32 B>
  BitVector<B> & BitVector<B>::operator=(const BitVector<B> & rhs)
  {
    memcpy(m_bits,rhs.m_bits,sizeof(m_bits));
    return *this;
  }

#endif // Fri Mar 13 16:04:59 2015 XXX TESTING GCC CODE GEN IMPACTS

  // This is the general case..
  template <u32 B>
  BitVector<B>::BitVector(const u32 value)
  {
    Clear();
    u32 startIdx = (u32) MAX(0, ((s32) B) - 32);
    u32 length = B - startIdx;
    Write(startIdx, length, value);
  }

  // ..but this one special case helps g++ codegen tremendously!
  template <>
  inline BitVector<32>::BitVector(const u32 value)
  {
    m_bits[0] = value;
  }

#if 0 // NOT IMPLEMENTED: UNCLEAR SEMANTICS
  template <u32 S>
  BitVector<S>::BitVector(const s32 value)
  {
    m_stg[WORDS-1] = (u32) value;
  }

  template <u32 S>
  BitVector<S>::BitVector(const u64 value)
  {
    m_stg[WORDS-1] = value;
  }

  template <u32 S>
  BitVector<S>::BitVector(const s64 value)
  {
    m_stg[WORDS-1] = (u64) value;
  }
#endif

  template <u32 B>
  void BitVector<B>::Clear()
  {
    memset(m_bits, 0, sizeof(m_bits));
  }

  /*
   * TODO These seem a little dumpy and slow. We could
   * probably make this implementation work with some
   * better bit-fu .
   *
   * SHAZAM! Slightly better bit-fu has arrived.  (Primarily in Read()
   * and Write(), to avoid going per-bit)
   */

  template <u32 B>
  void BitVector<B>::WriteBitUnsafe(const u32 idx, const bool bit)
  {
    u32 arrIdx = idx / BITS_PER_UNIT;
    u32 inIdx = idx % BITS_PER_UNIT;
    u32 newWord = 0x80000000 >> inIdx;

    if(!bit)
      m_bits[arrIdx] &= ~newWord;
    else
      m_bits[arrIdx] |= newWord;
  }

  template <u32 B>
  bool BitVector<B>::ReadBitUnsafe(const u32 idx) const
  {
    u32 arrIdx = idx / BITS_PER_UNIT;
    u32 intIdx = idx % BITS_PER_UNIT;
    return m_bits[arrIdx] & (0x80000000 >> intIdx);
  }

  template <u32 B>
  bool BitVector<B>::ToggleBitUnsafe(const u32 idx)
  {
    u32 arrIdx = idx / BITS_PER_UNIT;
    u32 inIdx = idx % BITS_PER_UNIT;
    u32 newWord = 0x80000000 >> inIdx;

    m_bits[arrIdx] ^= newWord;
    return m_bits[arrIdx] & newWord;
  }

  template <u32 B>
  void BitVector<B>::StoreBits(const u32 bits, const u32 startIdx, const u32 length)
  {
    if (!length) return;

    const u32 stopIdx = MIN((u32) B, startIdx + length) - 1;

    const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
    const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
    const bool firstUnitFull = firstUnitFirstBit == 0;

    const u32 lastUnitIdx = stopIdx / BITS_PER_UNIT;
    const u32 lastUnitLastBit = (stopIdx % BITS_PER_UNIT);
    const bool lastUnitFull = lastUnitLastBit == (BITS_PER_UNIT - 1);

    const bool hasMultipleUnits = lastUnitIdx > firstUnitIdx;

    if (!hasMultipleUnits) {
      WriteToUnit(firstUnitIdx, firstUnitFirstBit, length,
                  bits >> (BITS_PER_UNIT - (firstUnitFirstBit + length)));
      return;
    }

    // lastFullUnitIdx can go negative so be signed through here
    const s32 firstFullUnitIdx = firstUnitFull ? firstUnitIdx : firstUnitIdx + 1;
    const s32 lastFullUnitIdx = lastUnitFull ? lastUnitIdx : lastUnitIdx - 1;

    if (!firstUnitFull) {
      const u32 firstUnitLength = BITS_PER_UNIT - firstUnitFirstBit;
      WriteToUnit(firstUnitIdx, firstUnitFirstBit, firstUnitLength, bits);
    }

    for (s32 idx = firstFullUnitIdx; idx <= lastFullUnitIdx; ++idx)
      m_bits[idx] = bits;

    if (!lastUnitFull) {
      const u32 lastUnitLength = lastUnitLastBit + 1;
      WriteToUnit(lastUnitIdx, 0, lastUnitLength,
                  bits >> (BITS_PER_UNIT - lastUnitLength));
    }
  }

  template <u32 B>
  void BitVector<B>::Print(ByteSink & ostream) const
  {
    for (u32 i = 0; i < B; i += 4)
      ostream.Printf("%x",Read(i,4));
  }

  template <u32 B>
  void BitVector<B>::PrintBase2(ByteSink& ostream) const
  {
    for(u32 i = 0; i < B; i++)
    {
      ostream.Printf("%d", Read(i, 1) ? 1 : 0);
    }
  }

  template <u32 B>
  void BitVector<B>::PrintBytes(ByteSink& ostream) const
  {
    for(u32 w = 0; w < ARRAY_LENGTH; w++)
    {
      ostream.Print(m_bits[w], Format::BEU32);
    }
  }

  template <u32 B>
  bool BitVector<B>::Read(ByteSource & istream)
  {
    istream.SkipWhitespace();

    BitVector<B> temp;
    for (u32 i = 0; i < B; i += 4)
    {
      s32 hex;
      if (!istream.Scan(hex, Format::HEX, 1))
      {
        return false;
      }

      temp.Write(i, 4, hex);
    }

    *this = temp;
    return true;
  }

  template <u32 B>
  bool BitVector<B>::ReadBytes(ByteSource& istream)
  {
    BitVector<B> temp;
    for(u32 w = 0; w < ARRAY_LENGTH; w++)
    {
      if (!istream.Scan(temp.m_bits[w], Format::BEU32))
      {
        return false;
      }
    }
    *this = temp;
    return true;
  }

  template <u32 B>
  bool BitVector<B>::ReadBase2(ByteSource& istream)
  {
    istream.SkipWhitespace();

    BitVector<B> temp;
    for(u32 i = 0; i < B; i++)
    {
      s32 bit;
      if(!istream.Scan(bit, Format::BIN, 1))
      {
        return false;
      }

      temp.Write(i, 1, bit);
    }

    *this = temp;
    return true;
  }

  template <u32 B>
  bool BitVector<B>::operator==(const BitVector & rhs) const
  {
    return 0 == memcmp(m_bits, rhs.m_bits, sizeof(m_bits));

  }

  template <u32 B>
  void BitVector<B>::FromArray(const u32 array[ARRAY_LENGTH])
  {
    for(u32 i = 0; i < ARRAY_LENGTH; i++)
      m_bits[i] = array[i];
  }

  template <u32 B>
  void BitVector<B>::ToArray(u32 array[ARRAY_LENGTH]) const
  {
    for(u32 i = 0; i < ARRAY_LENGTH; i++)
      array[i] = m_bits[i];
  }

  template <u32 B>
  u32 BitVector<B>::PopulationCount(const u32 startIdx, const u32 len) const
  {
    const u32 end = MIN(B, startIdx + len);
    const u32 length = end - startIdx;

    if (length <= 32) return PopCount(Read(startIdx, length));

    // We are not going to end in the same word in which we started
    u32 ones = 0;
    u32 idx = startIdx / 32;

    if (startIdx % 32 != 0)
    {
      ones += PopCount(Read(startIdx, 32 - (startIdx % 32)));
      ++idx;
    }

    while (idx < end / 32)
      ones += PopCount(m_bits[idx++]);

    if (idx * 32 < end)
      ones += PopCount(Read(idx * 32, end - idx * 32));
    return ones;
  }


} /* namespace MFM */
