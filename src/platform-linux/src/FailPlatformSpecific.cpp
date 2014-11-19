#include "FailPlatformSpecific.h"
#include <stdlib.h>   /* For abort */

/* This declaration is in Fail.h but we currently aren't #including
   from platform-specific to core (because we are including the
   other way instead and don't want to risk looping just for
   this.) */
extern "C" const char * MFMFailCodeReason(int failCode) ;

extern "C" {

  static MFMErrorEnvironmentPointer_t mainThreadStackPtr = 0;

  /* All threads start out pointing at mainThreadStackPtr?  Then for
     threads running Tile's, the Tile initialization overwrites this
     to be Tile::m_errorEnvironmentStackTop, in
     Tile::ExecuteThreadHelper.  But what happens if some other
     non-Tile thread gets started?  We really need to use pthread_once
     and have its initialization routine check if mainThreadStackPtr
     has already been initialized? */

  __thread MFMErrorEnvironmentPointer_t * MFMPtrToErrEnvStackPtr = &mainThreadStackPtr;


  void MFMPrintErrorEnvironment(FILE * stream, MFMErrorEnvironmentPointer_t errenv) {
    volatile const char * file = errenv->file;
    int line = errenv->lineno;
    int code = errenv->thrown;
    if (!file) {
      file = "unknown";
      line = 0;
    }
    MFMPrintError(stream,const_cast<const char*>(file),line,code);
  }

  void MFMPrintError(FILE * stream, const char * file, const int line, const int code) {
    fprintf(stream,"%s:%d: FAILED: %s\n", file, line, MFMFailCodeReason(code));
  }
  void MFMFailHere(const char * file, const int line, int code) {
    MFMPrintError(stderr, file, line, code);
    fprintf(stderr, "\n\nUNCAUGHT FAILURE, ABORTING\n\n");
    abort();
  }

  void MFMLongJmpHere(jmp_buf buffer, const int toThrow) {
    longjmp(buffer,toThrow);
  }

  //  MFMErrorEnvironment * volatile MFMErrorEnvironmentStackTop = 0;

}
