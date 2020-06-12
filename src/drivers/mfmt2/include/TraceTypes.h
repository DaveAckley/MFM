/* -*- C++ -*- */
#ifndef TRACETYPES_H
#define TRACETYPES_H

#include <stdarg.h>

#include "dirdatamacro.h"

#include "itype.h"
#include "T2Types.h"
#include "Trace.h"
#include "Logger.h"

#define TLOG(LVL,FORMAT,...)                                        \
  do {                                                              \
    if (LOG.IfLog(Logger::LVL))                                     \
      TRACEPrintf(Logger::LVL, FORMAT , ## __VA_ARGS__ );           \
  } while (0)                                                       \


namespace MFM {

  const char * getTraceTypeType(TraceTypeCode ttc) ;
  const char * getTraceTypeBrief(TraceTypeCode ttc) ;
  const char * getTraceTypeName(TraceTypeCode ttc) ;

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD

  bool TRACEPrintf(Logger::Level level, const char * format, ...) ;

}

#endif /* TRACETYPES_H */
