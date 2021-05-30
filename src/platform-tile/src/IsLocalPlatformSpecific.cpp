#include "IsLocalPlatformSpecific.h"

/* At least with gcc-5_5.4.0 on ubuntu xenial, gcc sees through the
   'void * ret = &local; return ret;' code with a return-local-addr
   warning, which we promote to an error, which then kills the build.

   So ../Makefile is now compiling the files in this directory with
   '-Wno-error=return-local-addr'..
*/

void * __attribute__ ((noinline)) getStackBound()
{
  return __builtin_frame_address(0); // This seems tailor-made, no?
}
