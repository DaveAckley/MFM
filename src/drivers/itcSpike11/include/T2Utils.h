/* -*- C++ -*- */
#ifndef T2UTILS_H
#define T2UTILS_H

#include "FileByteSource.h"
#include "OverflowableCharBufferByteSink.h"
#include "dirdatamacro.h"
#include "Dirs.h"

namespace MFM {

  inline u8 mapDir8ToDir6(u8 dir8) {
    switch (dir8) {
    case Dirs::NORTH: 
    case Dirs::SOUTH: 
    default: return DIR_COUNT;
      
    case Dirs::NORTHEAST: return DIR_NE;
    case Dirs::EAST:      return DIR_ET;
    case Dirs::SOUTHEAST: return DIR_SE;
    case Dirs::SOUTHWEST: return DIR_SW;
    case Dirs::WEST:      return DIR_WT;
    case Dirs::NORTHWEST: return DIR_NW;
    }
  }

  inline u8 mapDir6ToDir8(u8 dir6) {
    switch (dir6) {
    default: FAIL(ILLEGAL_ARGUMENT);
      
    case DIR_NE: return Dirs::NORTHEAST;
    case DIR_ET: return Dirs::EAST;
    case DIR_SE: return Dirs::SOUTHEAST;
    case DIR_SW: return Dirs::SOUTHWEST;
    case DIR_WT: return Dirs::WEST;
    case DIR_NW: return Dirs::NORTHWEST;
    }
  }

  const char * getDir6Name(u32 dir6) ;
  bool readWholeFile(const char* path, ByteSink &to) ;
  bool readOneLinerFile(const char* path, ByteSink &to) ;
  bool readOneDecimalNumberFile(const char* path, s32 &to) ;
  bool readOneBinaryNumberFile(const char* path, s32 &to) ;
  bool readFloatsFromFile(const char* path, double * floats, u32 floatCount) ;

  bool readWholeProcessOutput(const char* cmd, ByteSink &to) ;

  u32 findPidOfProgram(const char * progname) ;

}

#endif /* T2UTILS_H */
