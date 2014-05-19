/* -*- C++ -*- */
#include "Fail.h"  /* For FAIL */
#include <string.h> /* For memset, memcpy */

namespace MFM {
  template <u32 BITS>
  BitVector<BITS>::BitVector()
  {
    Clear();
  }

  template <u32 BITS>
  BitVector<BITS>::BitVector(const u32 * const values)
  {
    memcpy(m_bits,values,sizeof(m_bits));
  }

  template <u32 BITS>
  BitVector<BITS>::BitVector(const BitVector & other)
  {
    memcpy(m_bits,other.m_bits,sizeof(m_bits));
  }

  template <u32 BITS>
  void BitVector<BITS>::Clear()
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

  template <u32 BITS>
  void BitVector<BITS>::WriteBit(int idx, bool bit)
  {
    int arrIdx = idx / BITS_PER_UNIT;
    int inIdx = idx % BITS_PER_UNIT;
    u32 newWord = 0x80000000 >> inIdx;

    if(!bit)
      m_bits[arrIdx] &= ~newWord;
    else
      m_bits[arrIdx] |= newWord;
  }

  template <u32 BITS>
  bool BitVector<BITS>::ToggleBit(int idx)
  {
    int arrIdx = idx / BITS_PER_UNIT;
    int inIdx = idx % BITS_PER_UNIT;
    u32 newWord = 0x80000000 >> inIdx;

    m_bits[arrIdx] ^= newWord;
    return m_bits[arrIdx] & newWord;
  }

  template <u32 BITS>
  bool BitVector<BITS>::ReadBit(int idx)
  {
    int arrIdx = idx / BITS_PER_UNIT;
    int intIdx = idx % BITS_PER_UNIT;

    return m_bits[arrIdx] & (0x80000000 >> intIdx);
  }

  template <u32 BITS>
  void BitVector<BITS>::Write(u32 startIdx,
                              u32 length,
                              u32 value)
  {
    if (length == 0)
      return;

    if (startIdx+length > BITS)
      FAIL(ILLEGAL_ARGUMENT);

    if (length > sizeof(BitUnitType) * CHAR_BIT)
      FAIL(ILLEGAL_ARGUMENT);

    /* Since we're writing no more than 32 bits into an array of 32 bit
       words, we can't need to touch more than two of them.  So unroll
       the loop.
    */

    const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
    const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
    const bool hasSecondUnit = (firstUnitFirstBit + length) > BITS_PER_UNIT;
    const u32 firstUnitLength = hasSecondUnit ? BITS_PER_UNIT-firstUnitFirstBit : length;

    WriteToUnit(firstUnitIdx, firstUnitFirstBit, firstUnitLength, value >> (length - firstUnitLength));

    if (hasSecondUnit)
      WriteToUnit(firstUnitIdx + 1, 0, length - firstUnitLength, value);
  }

#if 0
  template <u32 BITS>
  void BitVector<BITS>::Insert(u32 startIdx,
                               u32 length,
                               u32 value)
  {
    /* floor to a multiple of 32   vvv  */
    for(u32 i = ((BITS / BITS_PER_UNIT) * BITS_PER_UNIT) - 1;
        i >= startIdx + length; i--)
      {
        WriteBit(i, ReadBit(i - length));
      }
    Write(startIdx, length, value);
  }
#endif

  template <u32 BITS>
  u32 BitVector<BITS>::Read(const u32 startIdx, const u32 length) const
  {
    if (length == 0)
      return 0;

    if (startIdx+length > BITS)
      FAIL(ILLEGAL_ARGUMENT);

    if (length > sizeof(BitUnitType) * CHAR_BIT)
      FAIL(ILLEGAL_ARGUMENT);

    /* See Write(u32,u32,u32) for theory, such as it is */

    const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
    const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
    const bool hasSecondUnit = (firstUnitFirstBit + length) > BITS_PER_UNIT;
    const u32 firstUnitLength = hasSecondUnit ? BITS_PER_UNIT-firstUnitFirstBit : length;

    u32 ret = ReadFromUnit(firstUnitIdx, firstUnitFirstBit, firstUnitLength);

    if (hasSecondUnit) {
      const u32 secondUnitLength = length - firstUnitLength;
      ret = (ret << secondUnitLength) | ReadFromUnit(firstUnitIdx + 1, 0, secondUnitLength);
    }

    return ret;
  }

  template <u32 BITS>
  void BitVector<BITS>::Print(ByteSink & ostream) const
  {
    for (u32 i = 0; i < BITS; i += 4)
      ostream.WriteByte("0123456789abcdef"[Read(i,4)]);
  }

} /* namespace MFM */
