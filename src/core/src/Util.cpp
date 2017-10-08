#include "Util.h"
#include <time.h>  /* For nanosleep */

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

  u32 DigitCount64(u64 num, u32 base)
  {
    u32 count = 0;
    while(num)
    {
      count++;
      num /= base;
    }
    return count;
  }

  void IntAlphaEncode(u32 num, char* output)
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

  void Sleep(u32 seconds, u64 nanos)
  {
    struct timespec tspec;
    tspec.tv_sec = seconds;
    tspec.tv_nsec = nanos;

    nanosleep(&tspec, NULL);
  }

  u32 InterpolateColors(const u32 color1, const u32 color2, const u32 percentOfColor1)
  {
    if (percentOfColor1 >= 100) return color1;
    if (percentOfColor1 == 0) return color2;

    const u32 percentOfColor2 = 100 - percentOfColor1;
    u32 res = 0;
    for (u32 i = 0; i < 32; i += 8) {
      u32 comp1 = (color1 >> i) & 0xff;
      u32 comp2 = (color2 >> i) & 0xff;
      u32 mix = (percentOfColor1 * comp1 + percentOfColor2 * comp2) / 100;
      res |= mix << i;
    }

    return res;
  }

 const char * StripThroughHexSpaceIfExists(const char * str) 
  {
    if (str) {
      const char * p = str;
      char ch;
      while ((ch = *p++) != '%' && ch) { }
      if (ch && *p++ == '2' && *p++ == '0') return p;
    }
    return str;
  }

}
