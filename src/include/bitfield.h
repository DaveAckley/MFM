#ifndef BITFIELD_H      /* -*- C++ -*- */
#define BITFIELD_H

#include "itype.h"
#include <stdio.h>

#define BITFIELD_WORDSIZE 32

/* 
 * Try to make this a multiple of 32. Otherwise
 * there will be some wasted space.
 */
template <int bitLength>
class BitField
{
public:
  static const u32 ARRAY_LENGTH = (bitLength+BITFIELD_WORDSIZE-1) / BITFIELD_WORDSIZE;
private:
  u32 m_bits[ARRAY_LENGTH];

  u32 MakeMask(int pos, int len);

  void WriteBit(int idx, bool bit);

  bool ReadBit(int idx);
public:

  BitField();

  BitField(const BitField & other);

  BitField(u32* values);

  u32 Read(int startIdx, int length);

  void Write(int startIdx, int length, u32 value);

  /* 
   * Inserts a value at startIdx. Pushes the values
   * already there down length bits.
   */
  void Insert(int startIdx, int length, u32 value);

  /*
   * Removes length bits from this bitfield, then pushes
   * all bits to the right of them back length bits.
   */
  void Remove(int startIdx, int length);

  void Print(FILE* ostream);
			   
};

#include "bitfield.tcc"

#endif /*BITFIELD_H*/
