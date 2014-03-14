/* -*- C++ -*- */
#include "Fail.h"  /* For FAIL */
#include <string.h> /* For memset, memcpy */

namespace MFM {
template <u32 BITS>
BitVector<BITS>::BitVector()
{
  memset(m_bits, 0, sizeof(m_bits));
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
  const u32 firstUnitLength = hasSecondUnit ? length - (BITS_PER_UNIT-firstUnitFirstBit) : length;

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
  if (startIdx+length > BITS)
    FAIL(ILLEGAL_ARGUMENT);

  if (length > sizeof(BitUnitType) * CHAR_BIT)
    FAIL(ILLEGAL_ARGUMENT);

  /* See Write(u32,u32,u32) for theory, such as it is */

  const u32 firstUnitIdx = startIdx / BITS_PER_UNIT;
  const u32 firstUnitFirstBit = startIdx % BITS_PER_UNIT;
  const bool hasSecondUnit = (firstUnitFirstBit + length) > BITS_PER_UNIT;
  const u32 firstUnitLength = hasSecondUnit ? length - (BITS_PER_UNIT-firstUnitFirstBit) : length;

  u32 ret = ReadFromUnit(firstUnitIdx, firstUnitFirstBit, firstUnitLength);

  if (hasSecondUnit) {
    const u32 secondUnitLength = length - firstUnitLength;
    ret = (ret << secondUnitLength) | ReadFromUnit(firstUnitIdx + 1, 0, secondUnitLength);
  }

  return ret;
}

template <u32 BITS>
void BitVector<BITS>::Print(FILE* ostream) const
{
  for (u32 i = 0; i < BITS; i += 4) 
    fputc("0123456789abcdef"[Read(i,4)], ostream);
  /*
  u32 mask = 0xf0000000;
  for(u32 i = 0; i < ARRAY_LENGTH; i++)
  {
    for(int j = 0; j < 8; j++)
    {
      char c = (m_bits[i] & (mask >> (j << 2)))
		>> (28 - (j << 2));
      if(c >= 0xa)
      {
	fputc(c - 0xa + 'a', ostream);
      }
      else
      {
	fputc(c + '0', ostream);
      }
    }
    if(i + 1 < ARRAY_LENGTH)
    {
      fputc('-', ostream);
    }
  }
  */
}

/*
template<u32 BITS>
void BitVector<BITS>::Remove(u32 startIdx, u32 length)
{
  for(u32 i = startIdx; i < BITS - length; i++)
  {
    WriteBit(i, ReadBit(i + length));
  }
  for(u32 i = BITS - length; i < BITS; i++)
  {
    WriteBit(i, 0);
  }
}
*/


} /* namespace MFM */
