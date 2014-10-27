#ifndef FAILPLATFORMSPECIFIC_H      /* -*- C++ -*- */
#define FAILPLATFORMSPECIFIC_H

#include <setjmp.h>   /* For jmp_buf, setjmp, longjmp */
#include <stdio.h>    /* For FILE */
#include <pthread.h>  /* For __thread */

typedef struct MFMErrorEnvironment * volatile MFMErrorEnvironmentPointer_t;

struct MFMErrorEnvironment {
  jmp_buf buffer;               /* the system state as of the unwind_protect entry */
  volatile const char * file;   /* the file name of the original failure */
  volatile int lineno;          /* the line number of the original failure */
  volatile int thrown;          /* Return value(s) from setjmp call */
  MFMErrorEnvironmentPointer_t prev; /* Back link to previous error environment */
} ;


/* Pointer to thread-local error environment stack top */
extern "C"
  __thread MFMErrorEnvironmentPointer_t * MFMPtrToErrEnvStackPtr;

extern "C" void MFMPrintErrorEnvironment(FILE * stream, MFMErrorEnvironmentPointer_t errenv) ;

extern "C" void MFMPrintError(FILE * stream, const char * file, const int line, const int code) ;

extern "C" void MFMFailHere(const char * file, const int line, const int code) __attribute__ ((noreturn));
extern "C" void MFMLongJmpHere(jmp_buf buffer, const int toThrow) __attribute__ ((noreturn));
extern "C" const char * MFMFailCodeReason(int failCode) ;

#define MFM_FAIL_CODE_NUMBER(code) (MFM_FAIL_CODE_REASON_##code)

#define FAIL(code)                                                 \
  ((MFMPtrToErrEnvStackPtr && *MFMPtrToErrEnvStackPtr)?            \
   ((*MFMPtrToErrEnvStackPtr)->file = __FILE__,                    \
    (*MFMPtrToErrEnvStackPtr)->lineno = __LINE__,                  \
    MFMLongJmpHere((*MFMPtrToErrEnvStackPtr)->buffer,              \
                   MFM_FAIL_CODE_NUMBER(code)),0) :                \
   (MFMFailHere(__FILE__,__LINE__,                                 \
                MFM_FAIL_CODE_NUMBER(code)),0))

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
  unwindProtect_errorEnvironment.prev = (*MFMPtrToErrEnvStackPtr);	      \
  (*MFMPtrToErrEnvStackPtr) = &unwindProtect_errorEnvironment;                \
  if ((unwindProtect_errorEnvironment.thrown =			              \
       setjmp(unwindProtect_errorEnvironment.buffer)) != 0) {		      \
    /* Currently nothing special to do */                                     \
  } else {								      \
    {block}								      \
  }                                                                           \
  (*MFMPtrToErrEnvStackPtr) = (*MFMPtrToErrEnvStackPtr)->prev;                \
  if (unwindProtect_errorEnvironment.thrown) {				      \
    int MFMThrownFailCode __attribute__ ((unused)) =                          \
      unwindProtect_errorEnvironment.thrown;                                  \
    {cleanup}	                                                              \
  }                                                                           \
} while (0)


#endif /*FAILPLATFORMSPECIFIC_H*/

