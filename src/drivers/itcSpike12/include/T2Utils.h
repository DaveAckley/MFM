/* -*- C++ -*- */
#ifndef T2UTILS_H
#define T2UTILS_H

#include "FileByteSource.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {
  bool readWholeFile(const char* path, ByteSink &to) ;
  bool readOneLinerFile(const char* path, ByteSink &to) ;
  bool readOneDecimalNumberFile(const char* path, s32 &to) ;
  bool readFloatsFromFile(const char* path, double * floats, u32 floatCount) ;

  bool readWholeProcessOutput(const char* cmd, ByteSink &to) ;

  u32 findPidOfProgram(const char * progname) ;

  void printComma(u32 num, ByteSink & to) ;

}

#endif /* T2UTILS_H */
