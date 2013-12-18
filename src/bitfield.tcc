template <int bitLength>
BitField<bitLength>::BitField(Uint32* values)
{
  int bytes = bitLength >> 5;
  for(int i = 0; i < bytes; i++)
  {
    m_bits[i] = values[i];
  }
}

template<int bitLength>
Uint32 BitField<bitLength>::MakeMask(int pos, int len)
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

template <int bitLength>
Uint32 BitField<bitLength>::Write(int startIdx, int length, Uint32 value)
{
  int idx = startIdx >> 5;
  int topBits = (startIdx & 0x1f);

  return 0; /* Fix this! */
}

template <int bitLength>
Uint32 BitField<bitLength>::Read(int startIdx, int length)
{
  int idx = startIdx >> 5; /*The first box we will read from*/
  int topBits = (startIdx & 0x1f); /*The number of bits before the leading bit*/
  int startBit = 32 - topBits; /*The shift index of the first bit to read from*/
  int endBit = startBit + length; /*The shift index of the last bit to read from*/
  int secTop = length - startBit; /*The number of bits in the second int*/

  /* Are we only reading in one box? */
  if(secTop < 0)
  {
    return (m_bits[idx] & MakeMask(topBits, length)) >> (32 - length);
  }
  else
  {
    /*Let's make the ends of the read*/
    int top = m_bits[idx] & MakeMask(topBits, startBit);
    int bot = (m_bits[idx + 1] & MakeMask(0,  secTop)) >> (31 - secTop);
    return bot | (top << secTop);
  }
}


