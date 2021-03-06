/* -*- C++ -*- */
#ifndef T2UTILS_H
#define T2UTILS_H

#include <string.h> /*for strlen*/
#include <string>   /*for std::string*/
#include "FileByteSource.h"
#include "CharBufferByteSource.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {
  u32 getModTimeOfFile(const char* path) ;
  bool readWholeFile(const char* path, ByteSink &to) ;
  bool readOneLinerFile(const char* path, ByteSink &to) ;
  bool readOneDecimalNumberFile(const char* path, s32 &to) ;
  bool readFloatsFromFile(const char* path, double * floats, u32 floatCount) ;

  u64 digestWholeFile64(const char* path) ;
  bool digestWholeFile(const char* path, ByteSink& digestout, bool ashex) ;

  bool writeWholeFile(const char* path, ByteSource &from) ;
  bool writeWholeFile(const char* path, const char * data) ;

  bool readWholeProcessOutput(const char* cmd, ByteSink &to) ;

  u32 findPidOfProgram(const char * progname) ;

  void printComma(u32 num, ByteSink & to) ;
  void printComma(u64 num, ByteSink & to) ;

  const char * formatSize(u32 usize, bool trimwhite) ;

  bool endsWith(const std::string& str, const std::string& suffix) ;
}

#endif /* T2UTILS_H */
