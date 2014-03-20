/*                                              -*- mode:C++ -*-
  fail.h Support for meaningful runtime deaths when necessary
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file fail.h Support for meaningful runtime deaths when necessary
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef FAIL_H
#define FAIL_H

#include <setjmp.h>   /* For jmp_buf, setjmp, longjmp */

struct MFMErrorEnvironment {
  jmp_buf buffer;	      /* the system state as of the unwind_protect entry */
  const char * file;          /* the file name of the original failure */
  int lineno;                 /* the line number of the original failure */ 
  int thrown;                 /* Return value(s) from setjmp call */
  MFMErrorEnvironment * prev; /* Back link to previous error environment */
};

extern "C" MFMErrorEnvironment * MFMErrorEnvironmentStackTop;  /* Pointer to deepest open errenv */
extern "C" void MFMFailHere(const char * file, const int line, const int code) __attribute__ ((noreturn));
extern "C" const char * MFMFailCodeReason(int failCode) ;

#define MFM_FAIL_CODE_NUMBER(code) (MFM_FAIL_CODE_REASON_##code)

#define FAIL(code)                                                 \
  (MFMErrorEnvironmentStackTop?                                    \
   (MFMErrorEnvironmentStackTop->file = __FILE__,                  \
    MFMErrorEnvironmentStackTop->lineno = __LINE__,                \
    longjmp(MFMErrorEnvironmentStackTop->buffer,                   \
            MFM_FAIL_CODE_NUMBER(code)),0) :                       \
    (MFMFailHere(__FILE__,__LINE__,                                \
                 MFM_FAIL_CODE_REASON_UNCAUGHT_FAILURE),0))

#define XX(a) MFM_FAIL_CODE_REASON_##a,
enum MFMFailureCodes{
  ZERO_UNUSED = 0,
#include "FailCodes.h"
#undef XX
  MAX_FAILURE
};

/**
   Execute 'block', but if any FAIL()'s occur, stop executing 'block'
   at that point and execute 'cleanup'.

   NOTE WELL THE FOLLOWING WARNING ABOUT unwind_protect!
  
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 

   To avoid memory allocation and lots of extra C++ hair,
   unwind_protect (and FAIL) are based on the ancient crufty C
   setjmp/longjmp mechanism, rather than the modern slick C++
   try/catch mechanism.  

   That means the following text, from the longjmp(3) man page,
   applies here as well, reading 'unwind_protect' for 'setjmp' and
   'FAIL' for longjmp:

       The values of automatic variables are unspecified after a call
       to longjmp() if they meet all the following criteria:

       · they are local to the function that made the corresponding
          setjmp(3) call;

       · their values are changed between the calls to setjmp(3) and
          longjmp(); and

       · they are not declared as volatile.

   In practical terms, that means the output of code like:

     int num = 0;
     unwind_protect({ },{ num = 1; FAIL(ILLEGAL_STATE); });
     printf("%d\n",num);

   is UNSPECIFIED.  In anecdotal tests, it tends to print 1 if
   debugging is enabled, and tends to print 0 if optimization is
   enabled, consistent with expectations about whether the compiler
   decided to allocate num to a register --- but no specific
   conditions can be assumed or counted upon.

   To avoid problems like this, it is recommended that the 'block' in
   an unwind_protect consist essentially entirely of a function call
   to something else that does the work, and possibly FAILS, without
   affecting the local variables of the caller.

   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 

 */
#define unwind_protect(cleanup,block)                                         \
do {									      \
  MFMErrorEnvironment unwindProtect_errorEnvironment;			      \
  unwindProtect_errorEnvironment.prev = MFMErrorEnvironmentStackTop;	      \
  MFMErrorEnvironmentStackTop = &unwindProtect_errorEnvironment;	      \
  if ((unwindProtect_errorEnvironment.thrown =			              \
       setjmp(unwindProtect_errorEnvironment.buffer)) != 0) {		      \
    /* Currently nothing special to do */                                     \
  } else {								      \
    {block}								      \
  }                                                                           \
  MFMErrorEnvironmentStackTop = MFMErrorEnvironmentStackTop->prev;	      \
  if (unwindProtect_errorEnvironment.thrown) {				      \
    int MFMThrownFailCode __attribute__ ((unused)) =                          \
      unwindProtect_errorEnvironment.thrown;                                  \
    {cleanup}	                                                              \
  }                                                                           \
} while (0)


#endif  /* FAIL_H */

