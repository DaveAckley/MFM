#ifndef ISLOCALPLATFORMSPECIFIC_H      /* -*- C++ -*- */
#define ISLOCALPLATFORMSPECIFIC_H

#include <stdlib.h>  /* for abort() */

/*
  Compute a number 'somewhat' below the current stack pointer.
  getStackBound() works hard to avoid being inlined, and then returns
  the address of a local variable within the stack frame of the
  getStackBound call itself, which should, in a sane universe, be
  'slightly but not that much' beyond the end of the caller's stack
  frame.
 */
void * __attribute__ ((noinline)) getStackBound() ;

/*
   Determine if a given address lies in the local variable space of
   the current function's stack frame.

   Note this method may return true for some addresses that don't
   actually contain local variables, due to stack alignment padding or
   who-knows-what.  But we are only concerned with addresses that _do_
   land in the storage space of local variables, and we don't mind some
   false positives nearby.

   (At least, that's what we're hoping for.)

 */

inline bool _IsLocal(void * address) __attribute__ ((always_inline));

bool _IsLocal(void * address)
{
  // Here we assume the stack grows down.  Can arrange to establish
  // the stack direction during runtime startup if that's uncertain.
  void * highest = __builtin_frame_address(0);

  if (highest == 0)
  {
    // 0 apparently amounts to some kind of error condition from
    // __builtin_frame_address
    abort();
  }

  // We want to access the stack pointer to find the lower bound on
  // the current stack frame.  Now, conceivably, culam might be able
  // to know this directly itself, because any and all variables in
  // the local stack frame were put there by culam.  But that's's at
  // least non-obvious, depending on how gcc lays out scoped local
  // variables and so forth.  Instead, we make another function,
  // 'getStackBound' -- as non-inlinable as we can make it -- which
  // returns the address of a local variable within it, and that
  // should serve as an overestimate of the top of the stack inside
  // us.

  void * lowest = getStackBound();

  // OK, finally the actual test.  If the given address is between
  // lowest and highest, we say it _IsLocal in the current stack frame.
  return ((address >= lowest) && (address <= highest));
}



#endif /*ISLOCALPLATFORMSPECIFIC_H*/
