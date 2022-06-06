#ifndef FAILPLATFORMSPECIFIC_H      /* -*- C++ -*- */
#define FAILPLATFORMSPECIFIC_H

#include <setjmp.h>   /* For jmp_buf, setjmp, longjmp */
#include <stdio.h>    /* For FILE */
#include <pthread.h>  /* For __thread */
#include <execinfo.h> /* For backtrace */
#include <exception>  /* For std::exception */

typedef struct MFMErrorEnvironment * volatile MFMErrorEnvironmentPointer_t;

#define MAX_BACKTRACE_LEVELS 25
struct MFMErrorEnvironment {
  jmp_buf buffer;               /* the system state as of the unwind_protect entry */
  volatile const char * file;   /* the file name of the original failure */
  volatile int lineno;          /* the line number of the original failure */
  volatile int thrown;          /* Return value(s) from setjmp call */
  void * backtraceArray[MAX_BACKTRACE_LEVELS]; /* Where we were when we threw */
  unsigned backtraceSize;       /* Number of entries used in backtraceArray */
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
  FAIL_BY_NUMBER(MFM_FAIL_CODE_NUMBER(code))

#define FAIL_BY_NUMBER_OLD(number)                                     \
  ((MFMPtrToErrEnvStackPtr && *MFMPtrToErrEnvStackPtr)?            \
   ((*MFMPtrToErrEnvStackPtr)->file = __FILE__,                    \
    (*MFMPtrToErrEnvStackPtr)->lineno = __LINE__,                  \
    (*MFMPtrToErrEnvStackPtr)->backtraceSize = 0,                  \
  /*  (*MFMPtrToErrEnvStackPtr)->backtraceSize =                   \
      backtrace((*MFMPtrToErrEnvStackPtr)->backtraceArray,         \
      MAX_BACKTRACE_LEVELS), */                                    \
    MFMLongJmpHere((*MFMPtrToErrEnvStackPtr)->buffer,              \
                   number),0) :                                    \
   (MFMFailHere(__FILE__,__LINE__,                                 \
                number),0))

struct FailException : public std::exception {
  const int mCode;
  const char * mFile;
  const int mLine;
  void * mBacktraceArray[MAX_BACKTRACE_LEVELS]; /* Where we were when we threw */
  unsigned mBacktraceSize;       /* Number of entries used in backtraceArray */

  FailException(int code, const char * file, int line)
    : mCode(code)
    , mFile(file)
    , mLine(line)
  {
    mBacktraceSize = backtrace(mBacktraceArray, MAX_BACKTRACE_LEVELS); 
  }
  virtual const char * what() const throw() {
    return "failException";
  }
  void prettyPrint(FILE * stream) const ;
};

extern "C" void MFMPrintFailException(FILE * stream, const FailException & fe) ;

extern "C" void MFMThrowHere(const FailException & fe) __attribute__ ((noreturn));


extern "C" typedef void (*MFMFailHook)(const FailException & fe, const char * unwindFile, int unwindLine);

extern "C" MFMFailHook MFMUnwindProtectLoggingHook;

#define FAIL_BY_NUMBER(number)                  \
  MFMThrowHere(FailException(number,__FILE__,__LINE__))

#define unwind_protect(cleanup,block)                                   \
  do {                                                                  \
    try { block }                                                       \
    catch (FailException & _fe) {                                       \
      const FailException &                                                \
        unwindProtect_FailException __attribute__ ((unused)) = _fe;        \
      const int MFMThrownFailCode __attribute__ ((unused)) = _fe.mCode;    \
      const char * MFMThrownFromFile __attribute__ ((unused)) = _fe.mFile; \
      const int MFMThrownFromLineNo __attribute__ ((unused)) = _fe.mLine;  \
      void * const * MFMThrownBacktraceArray __attribute__ ((unused)) =    \
        _fe.mBacktraceArray;                                               \
      unsigned MFMThrownBacktraceSize __attribute__ ((unused)) =           \
        _fe.mBacktraceSize;                                                \
      if (MFMUnwindProtectLoggingHook != NULL)                             \
        (*MFMUnwindProtectLoggingHook)(_fe,__FILE__,__LINE__);             \
      { cleanup }                                                          \
    }                                                                      \
  } while (0)

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
#define unwind_protect_OLD(cleanup,block)                                         \
do {									      \
  MFMErrorEnvironment unwindProtect_errorEnvironment;			      \
  unwindProtect_errorEnvironment.prev = (*MFMPtrToErrEnvStackPtr);	      \
  (*MFMPtrToErrEnvStackPtr) = &unwindProtect_errorEnvironment;                \
  unwindProtect_errorEnvironment.thrown = setjmp(unwindProtect_errorEnvironment.buffer); \
  if (__builtin_expect(unwindProtect_errorEnvironment.thrown,0)) {            \
    /* something FAILed, but nothing special to do here */                    \
  } else {                                                                    \
    {block}								      \
  }                                                                           \
  (*MFMPtrToErrEnvStackPtr) = (*MFMPtrToErrEnvStackPtr)->prev;                \
  if (__builtin_expect(unwindProtect_errorEnvironment.thrown,0)) {            \
    int MFMThrownFailCode __attribute__ ((unused)) =                          \
      unwindProtect_errorEnvironment.thrown;                                  \
    const char * MFMThrownFromFile __attribute__ ((unused)) =                 \
      (const char *) unwindProtect_errorEnvironment.file;                     \
    unsigned MFMThrownFromLineNo __attribute__ ((unused)) =                   \
      unwindProtect_errorEnvironment.lineno;                                  \
    void * const * MFMThrownBacktraceArray __attribute__ ((unused)) =         \
      unwindProtect_errorEnvironment.backtraceArray;                          \
    unsigned MFMThrownBacktraceSize __attribute__ ((unused)) =                \
      unwindProtect_errorEnvironment.backtraceSize;                           \
    {cleanup}	                                                              \
  }                                                                           \
} while (0)


#endif /*FAILPLATFORMSPECIFIC_H*/
