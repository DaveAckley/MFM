#include "Util.h"

namespace MFM
{
  u32 DigitCount(u32 num, u32 base)
  {
    u32 count = 0;
    while(num)
    {
      count++;
      num /= base;
    }
    return count;
  }

  void IntLexEncode(u32 num, char* output)
  {
    u32 digits = DigitCount(num, 26);
    output[digits] = 0;

    /* Off by one case for the lazy*/
    if(!num)
    {
      output[0] = 'a';
      output[1] = 0;
    }
    else
    {
      while(num)
      {
	output[--digits] = 'a' + num % 26;
	num /= 26;
      }
    }
  }
}
