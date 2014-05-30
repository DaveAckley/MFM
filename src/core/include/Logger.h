#ifndef LOGGER_H      /* -*- C++ -*- */
#define LOGGER_H

#include "itype.h"
#include "ByteSink.h"
#include <stdarg.h>

namespace MFM {

  class Logger
  {
  public:
    enum Level {
      NONE,      // 'NONE' must remain 0 to avoid races from logging in static ctors
      ERROR,
      WARNING,
      MESSAGE,
      DEBUG,
      DEBUG1,
      DEBUG2,
      DEBUG3,
      ALL,

      MIN = NONE,
      MAX = ALL
    };

    static const char * StrLevel(Level l) {
      switch (l) {
      case NONE: return "NON";
      case ERROR: return "ERR";
      case WARNING: return "WRN";
      case MESSAGE: return "MSG";
      case DEBUG: return "DBG";
      case DEBUG1: return "DB1";
      case DEBUG2: return "DB2";
      case DEBUG3: return "DB3";
      case ALL: return "ALL";
      default: return "UNK";
      }
    }

    static bool ValidLevel(s32 levelNumber) {
      return levelNumber >= MIN && levelNumber <= MAX;
    }

    Level GetLevel() const {
      return m_logLevel;
    }

    Level SetLevel(u32 newLevel)
    {
      return SetLevel((Level)newLevel);
    }

    Level SetLevel(Level newLevel) {
      if (!ValidLevel(newLevel))
        FAIL(ILLEGAL_ARGUMENT);

      if (newLevel == m_logLevel)
        return m_logLevel;

      Level oldLevel = m_logLevel;

      Message("[%s->%s]",StrLevel(m_logLevel),StrLevel(newLevel));
      m_logLevel = newLevel;
      Message("[%s]",StrLevel(m_logLevel));

      return oldLevel;
    }

    ByteSink * SetByteSink(ByteSink & byteSink) {
      ByteSink * old = m_sink;
      m_sink = &byteSink;
      return old;
    }

    Logger(ByteSink & sink, Level initialLevel) :
      m_sink(&sink),
      m_logLevel(initialLevel),
      m_timeStamper(&m_defaultTimeStamper)
    { }

    bool IfLog(Level level) { return m_logLevel > NONE && level <= m_logLevel; }

    bool Log(Level level, const char * format, ... ) {
      va_list ap;
      va_start(ap, format);
      Vreport(level, format, ap);
      va_end(ap);
      return true;
    }

    void Vreport(Level level, const char * format, va_list & ap) {
      if (IfLog(level)) {
        m_sink->Printf("%@%s: ",m_timeStamper, StrLevel(level));
        m_sink->Vprintf(format, ap);
        m_sink->Println();
      }
    }

    void Error(const char * format, ... ) {
      va_list ap;
      va_start(ap, format);
      Vreport(ERROR, format, ap);
      va_end(ap);
    }

    void Warning(const char * format, ... ) {
      va_list ap;
      va_start(ap, format);
      Vreport(WARNING, format, ap);
      va_end(ap);
    }

    void Message(const char * format, ... ) {
      va_list ap;
      va_start(ap, format);
      Vreport(MESSAGE, format, ap);
      va_end(ap);
    }

    void Debug(const char * format, ... ) {
      va_list ap;
      va_start(ap, format);
      Vreport(DEBUG, format, ap);
      va_end(ap);
    }

    void SetTimeStamper(ByteSinkable * stamper) {
      m_timeStamper = stamper? stamper : &m_defaultTimeStamper;
      m_defaultTimeStamper.Reset();
    }

  private:
    ByteSink * m_sink;
    Level m_logLevel;

    class DefaultTimeStamper : public ByteSinkable {
      u32 m_calls;
    public:
      DefaultTimeStamper() : m_calls(0) { }
      void Reset() { m_calls = 0; }
      virtual void PrintTo(ByteSink & byteSink, s32 argument = 0) {
        byteSink.Print(++m_calls, byteSink.LEX32);
        byteSink.Print(": ");
      }
    } m_defaultTimeStamper;
    ByteSinkable * m_timeStamper;

  };

  extern Logger LOG;

} /* namespace MFM */

#endif /*LOGGER_H*/
