#include "IsLocalPlatformSpecific.h"

#include <stdint.h> /* for intptr_t, grrr */

void * __attribute__ ((noinline)) getStackBound()
{
  /* gcc-5_5.4.0 sees through the 'void * ret = &local; return ret;'
     code that was previously here.  Grrr.  So we're trying a
     roundtrip through intptr_t, despite its problems.  (Discovered on
     ubuntu xenial.)
  */
  char local;
  intptr_t obscure = (intptr_t) &local;
  asm(""); // try to block compiler optimizations
  return (void *) obscure;
}
