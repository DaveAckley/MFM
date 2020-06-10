#include "T2Constants.h"
#include "Fail.h"
#include "Logger.h"

namespace MFM {
  void vlog(Logger::Level level, const char * fmt, va_list ap) {
    if (LOG.IfLog(level)) {
      const u32 BUF_SIZE = 200;
      char buf[BUF_SIZE+1];
      vsnprintf(buf,BUF_SIZE,fmt,ap);
      buf[BUF_SIZE] = '\0';
      LOG.Log(level,"%s",buf);
    }
  }

  void vout(LineCountingByteSource & lcbs, Logger::Level level, const char * fmt, va_list ap) {
    const u32 BUF_SIZE = 500;
    char buf[BUF_SIZE+1];
    vsnprintf(buf,BUF_SIZE,fmt,ap);
    buf[BUF_SIZE] = '\0';
    lcbs.Msg(level,"%s",buf);
  }

  void debug(const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vlog(Logger::DEBUG, msg, ap);
    va_end(ap);
  }

  void debug(LineCountingByteSource& lcbs, const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vout(lcbs, Logger::DEBUG, msg, ap);
    va_end(ap);
  }

  void message(const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vlog(Logger::MESSAGE, msg, ap);
    va_end(ap);
  }

  void message(LineCountingByteSource& lcbs, const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vout(lcbs, Logger::MESSAGE, msg, ap);
    va_end(ap);
  }

  void warn(const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vlog(Logger::WARNING, msg, ap);
    va_end(ap);
  }

  void warn(LineCountingByteSource& lcbs, const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vout(lcbs, Logger::WARNING, msg, ap);
    va_end(ap);
  }

  void error(const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vlog(Logger::ERROR, msg, ap);
    va_end(ap);
  }

  void error(LineCountingByteSource& lcbs, const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vout(lcbs, Logger::ERROR, msg, ap);
    va_end(ap);
  }

  void fatal(const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vlog(Logger::ERROR, msg, ap);
    va_end(ap);
    FAIL(DESCRIBED_FAILURE);
  }

  void fatal(LineCountingByteSource& lcbs, const char * msg, ...) {
    va_list ap;
    va_start(ap,msg);
    vout(lcbs, Logger::ERROR, msg, ap);
    va_end(ap);
    FAIL(DESCRIBED_FAILURE);
  }

  const char * resourceTypeName(ResourceType rt) {
    switch (rt) {
    default: return "Unknown resource type";
#define XX(a) case RT_##a: return #a;
#define YY 
#define ZZ
      ALL_RESOURCE_TYPES()                       
#undef XX
#undef YY
#undef ZZ
    }
  }

  const char * resourceLevelName(ResourceLevel rl) {
    switch (rl) {
    default: return "Unknown resource level";
#define XX(a) case RL_##a: return #a;
#define YY 
#define ZZ
      ALL_RESOURCE_LEVELS()                       
#undef XX
#undef YY
#undef ZZ
    }
  }
}

