/* -*- C++ -*- */
#ifndef T2UTILS_H
#define T2UTILS_H

#include <string.h> /*for strlen*/
#include "FileByteSource.h"
#include "CharBufferByteSource.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {
  u32 getModTimeOfFile(const char* path) ;
  bool readWholeFile(const char* path, ByteSink &to) ;
  bool readOneLinerFile(const char* path, ByteSink &to) ;
  bool readOneDecimalNumberFile(const char* path, s32 &to) ;
  bool readFloatsFromFile(const char* path, double * floats, u32 floatCount) ;

  bool writeWholeFile(const char* path, ByteSource &from) ;
  bool writeWholeFile(const char* path, const char * data) ;

  bool readWholeProcessOutput(const char* cmd, ByteSink &to) ;

  u32 findPidOfProgram(const char * progname) ;

  void printComma(u32 num, ByteSink & to) ;
  void printComma(u64 num, ByteSink & to) ;

}

#endif /* T2UTILS_H */
