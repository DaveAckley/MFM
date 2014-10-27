extern "C" {
#include <stdlib.h>  /* for abort() */
#include <stdio.h>  /* for fprintf, stderr */
#include "Fail.h"

  static const char * FailStrings[] = {
    "zero unused",
#define XX(x) #x, 
#include "FailCodes.h"
#undef XX
    "max failure code"
  };

  const char * MFMFailCodeReason(int failCode) {
    if (failCode < 0 || failCode >= (int) (sizeof(FailStrings)/sizeof(FailStrings[0])))
      return "[failCode out of range]";
    return FailStrings[failCode];
  }

}
