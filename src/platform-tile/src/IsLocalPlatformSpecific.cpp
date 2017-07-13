#include "IsLocalPlatformSpecific.h"

void * __attribute__ ((noinline)) getStackBound()
{
  char local;
  void * ret = &local;
  asm(""); // try to block compiler optimizations
  return ret;
}
