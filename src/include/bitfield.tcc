
template <int bitLength>
BitField<bitLength>::BitField()
{
  for(u32 i = 0; i < ARRAY_LENGTH; i++)
  {
    m_bits[i] = 0;
  }
}

template <int bitLength>
BitField<bitLength>::BitField(u32* values)
{
  for(u32 i = 0; i < ARRAY_LENGTH; i++)
  {
    m_bits[i] = values[i];
  }
}

template <int bitLength>
BitField<bitLength>::BitField(const BitField & other)
{
  for(u32 i = 0; i < ARRAY_LENGTH; i++)
  {
    m_bits[i] = other.m_bits[i];
  }
}

template<int bitLength>
u32 BitField<bitLength>::MakeMask(int pos, int len)
{
  int maskAcc = 0;
  int maskBit = 0x80000000 >> pos;
  for(int i = 0; i < len; i++)
  {
    maskAcc |= maskBit;
    maskBit >>= 1;
  }
  return maskAcc;
}

/* 
 * TODO These seem a little dumpy and slow. We could
 * probably make this implementation work with some
 * better bit-fu .
 */
template <int bitLength>
void BitField<bitLength>::WriteBit(int idx, bool bit)
{
  int arrIdx = idx / BITFIELD_WORDSIZE;
  int inIdx = idx & 0x1f;
  u32 newWord = 0x80000000 >> inIdx;

  if(m_bits[arrIdx] & newWord)
  {
    if(!bit)
    {
      m_bits[arrIdx] &= (~newWord);
    }
  }
  else if(bit)
  {
    m_bits[arrIdx] |= newWord;
  }
}

template <int bitLength>
bool BitField<bitLength>::ReadBit(int idx)
{
  int arrIdx = idx / BITFIELD_WORDSIZE;
  int intIdx = idx % BITFIELD_WORDSIZE;
  
  return m_bits[arrIdx] & (0x80000000 >> intIdx);
}

template <int bitLength>
void BitField<bitLength>::Write(int startIdx,
				int length,
				u32 value)
{
  int mask = 1;
  for(int i = startIdx + length - 1; 
      i >= startIdx; i--)
  {
    WriteBit(i, value & mask);
    mask <<= 1;
  }
}

template <int bitLength>
void BitField<bitLength>::Insert(int startIdx,
				 int length,
				 u32 value)
{
  /* floor to a multiple of 32   vvv  */
  for(int i = ((bitLength / BITFIELD_WORDSIZE)
	       * BITFIELD_WORDSIZE) - 1;
      i >= startIdx + length; i--)
  {
    WriteBit(i, ReadBit(i - length));
  }
  Write(startIdx, length, value);
}

template <int bitLength>
u32 BitField<bitLength>::Read(int startIdx,
			      int length)
{
  u32 acc = 0;
  for(int i = startIdx; i < startIdx + length; i++)
  {
    acc <<= 1;
    acc |= ReadBit(i) ? 1 : 0;
  }
  return acc;
}

template <int bitLength>
void BitField<bitLength>::Print(FILE* ostream)
{
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
}

template<int bitLength>
void BitField<bitLength>::Remove(int startIdx, int length)
{
  for(int i = startIdx; i < bitLength - length; i++)
  {
    WriteBit(i, ReadBit(i + length));
  }
  for(int i = bitLength - length; i < bitLength; i++)
  {
    WriteBit(i, 0);
  }
}
