/*                                              -*- mode:C++ -*-
  Logger.h Configurable output logging system
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Logger.h Configurable output logging system
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef LOGGER_H
#define LOGGER_H

#include "itype.h"
#include "ByteSink.h"
#include "ByteSerializable.h"
#include "Mutex.h"
#include <stdarg.h>

#define MFM_LOG_DBG3(args) do {if (LOG.IfLog((Logger::Level) 3)) {LOG.Message args ;}} while (0)
#define MFM_LOG_DBG4(args) do {if (LOG.IfLog((Logger::Level) 4)) {LOG.Debug args ;}} while (0)
#define MFM_LOG_DBG5(args) do {if (LOG.IfLog((Logger::Level) 5)) {LOG.Debug args ;}} while (0)
#define MFM_LOG_DBG6(args) do {if (LOG.IfLog((Logger::Level) 6)) {LOG.Debug args ;}} while (0)
#define MFM_LOG_DBG7(args) do {if (LOG.IfLog((Logger::Level) 7)) {LOG.Debug args ;}} while (0)

namespace MFM
{

  /**
   * A logging system used for logging different kinds of messages to
   * a ByteSink .
   */
  class Logger
  {
  public:

    /**
     * An enumeration of all levels at which this Logger may be
     * elevated to . Lower levels are reserved for more severe messages .
     */
    enum Level
    {
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

    /**
     * Translates a Level to an immutable string .
     *
     * @param l The level to translate to a string .
     *
     * @returns The string which represents \c l .
     */
    static const char * StrLevel(Level l)
    {
      switch (l)
      {
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

    /**
     * Checks to see if a specified level number represents a Level in
     * this Logger .
     *
     * @param levelNumber The number of the level to check for
     *                    validity .
     *
     * @returns \c true if \c levelNumber represents a valid logging Level .
     */
    static bool ValidLevel(s32 levelNumber)
    {
      return levelNumber >= MIN && levelNumber <= MAX;
    }

    /**
     * Gets the current Level that this Logger is operating at .
     *
     * @returns The current Level that this Logger is operating at .
     */
    Level GetLevel() const
    {
      return m_logLevel;
    }

    /**
     * Sets the Level that this Logger will operate at by specifying
     * the number of the level to be set.
     *
     * @param newLevel the number of the Level to set this Logger
     *                 to. This FAILs with ILLEGAL_ARGUMENT if this is
     *                 not a valid Level number.
     *
     * @returns the Level that this Logger was operating at before
     *          this method was called.
     */
    Level SetLevel(u32 newLevel)
    {
      return SetLevel((Level)newLevel);
    }

    /**
     * Sets the Level that this Logger will operate at.
     *
     * @param newLevel The Level that this Logger will operate
     *                 at. This FAILs with ILLEGAL_ARGUMENT if this is
     *                 not a valid Level .
     *
     * @returns the Level that this Logger was operating at before
     *          this method was called.
     */
    Level SetLevel(Level newLevel)
    {
      if (!ValidLevel(newLevel))
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      if (newLevel == m_logLevel)
      {
        return m_logLevel;
      }

      Level oldLevel = m_logLevel;

      Debug("[%s->%s]",StrLevel(m_logLevel),StrLevel(newLevel));
      m_logLevel = newLevel;
      Debug("[%s]",StrLevel(m_logLevel));

      return oldLevel;
    }

    /**
     * Sets the ByteSink that this Logger will begin writing to.
     *
     * @param byteSink The ByteSink that this Logger will begin writing to.
     *
     * @returns A pointer to the ByteSink that this Logger was writing
     *          to before this method was called.
     */
    ByteSink * SetByteSink(ByteSink & byteSink)
    {
      ByteSink * old = m_sink;
      m_sink = &byteSink;
      return old;
    }

    /**
     * Constructs a new Logger which writes to a specified ByteSink
     * and logs at a particular logging Level .
     *
     * @param sink The ByteSink that this Logger will begin writing to.
     *
     * @param initialLevel The Level that this Logger will begin writing at.
     */
    Logger(ByteSink & sink, Level initialLevel) :
      m_sink(&sink),
      m_logLevel(initialLevel),
      m_timeStamper(&m_defaultTimeStamper)
    {
    }

    ~Logger()
    {
    }

    /**
     * Checks to see if this Logger is operating at a higher Level
     * than a specified one.
     *
     * @param level The Level used to test the elevation of this Logger .
     *
     * @returns \c true if this Logger is operating on some level
     *          higher than \c level , else \c false .
     */
    bool IfLog(Level level)
    {
      return m_logLevel > NONE && level <= m_logLevel;
    }

    /**
     * Logs a formatted message at a specified logging Level .
     *
     * @param level The logging Level to log this message at.
     *
     * @param format The format string used to parse the variadic
     *               arguments which follow it.
     *
     * @returns \c true .
     */
    bool Log(Level level, const char * format, ... )
    {
      va_list ap;
      va_start(ap, format);
      Vreport(level, format, ap);
      va_end(ap);
      return true;
    }

    /**
     * Logs a formatted message at a specified logging level .
     *
     * @param level The logging Level to log this message at.
     *
     * @param format The format string used to parse the variadic
     *               argument list .
     *
     * @param ap The variadic argument list describing the message to
     *           be logged.
     */
    void Vreport(Level level, const char * format, va_list & ap)
    {
      if (IfLog(level))
      {
        Mutex::ScopeLock lock(m_mutex); // Hold lock for this block

        m_sink->Printf("%@%s: ",m_timeStamper, StrLevel(level));
        m_sink->Vprintf(format, ap);
        m_sink->Println();
      }
    }

    /**
     * Logs a formatted message at the ERROR logging Level .
     *
     * @param format The format string used to parse the variadic
     *               arguments which follow it.
     */
    void Error(const char * format, ... )
    {
      va_list ap;
      va_start(ap, format);
      Vreport(ERROR, format, ap);
      va_end(ap);
    }

    /**
     * Logs a formatted message at the WARNING logging Level .
     *
     * @param format The format string used to parse the variadic
     *               arguments which follow it.
     */
    void Warning(const char * format, ... )
    {
      va_list ap;
      va_start(ap, format);
      Vreport(WARNING, format, ap);
      va_end(ap);
    }

    /**
     * Logs a formatted message at the MESSAGE logging Level .
     *
     * @param format The format string used to parse the variadic
     *               arguments which follow it.
     */
    void Message(const char * format, ... )
    {
      va_list ap;
      va_start(ap, format);
      Vreport(MESSAGE, format, ap);
      va_end(ap);
    }

    /**
     * Logs a formatted message at the DEBUG logging Level .
     *
     * @param format The format string used to parse the variadic
     *               arguments which follow it.
     */
    void Debug(const char * format, ... )
    {
      va_list ap;
      va_start(ap, format);
      Vreport(DEBUG, format, ap);
      va_end(ap);
    }

    /**
     * Sets the Time Stamper to be used by this Logger , a device used
     * to keep track of the number of messages that have been logged.
     *
     * @param stamper The ByteSerializable that will be printed as a
     *                prefix to each logging message.
     */
    void SetTimeStamper(ByteSerializable * stamper)
    {
      m_timeStamper = stamper? stamper : &m_defaultTimeStamper;
      m_defaultTimeStamper.Reset();
    }

  private:
    ByteSink * m_sink;
    Level m_logLevel;

    /**
     * A lock to ensure only one thread does logging at a time; the
     * underlying ByteSink routines are not thread-safe.
     */
    Mutex m_mutex;

    class DefaultTimeStamper : public ByteSerializable
    {
      u32 m_calls;
    public:
      DefaultTimeStamper() : m_calls(0) { }
      void Reset() { m_calls = 0; }
      virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0)
      {
        byteSink.Print(++m_calls, Format::LEX32);
        byteSink.Print(": ");
        return SUCCESS;
      }
      virtual Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
      {
        return UNSUPPORTED;
      }
    } m_defaultTimeStamper;
    ByteSerializable * m_timeStamper;

  };

  extern Logger LOG;

} /* namespace MFM */

#endif /*LOGGER_H*/
