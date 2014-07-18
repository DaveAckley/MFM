/*                                              -*- mode:C++ -*-
  LineCountingByteSource.h Wrapper class to count line and byte positions
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
  \file LineCountingByteSource.h Wrapper class to count line and byte positions
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef LINECOUNTINGBYTESOURCE_H
#define LINECOUNTINGBYTESOURCE_H

#include "itype.h"
#include "Fail.h"
#include "Logger.h"        /* For Logger::Level */
#include "ByteSource.h"

namespace MFM
{
  /**
   * A ByteSource that tracks how many lines of text have been read,
   * and what byte of the current line was most recently read.  Useful
   * for providing feedback to help pinpoint errors in ByteSources.
   */
  class LineCountingByteSource : public ByteSource
  {
   public:
    /**
     * Construct a new LineCountingByteSource which is not attached to
     * another ByteSource and is notready for use.
     */
    LineCountingByteSource() :
      m_bs(NULL),
      m_errs(&DevNull),
      m_label("unknown source"),
      m_lineNum(1),
      m_byteNum(0)
    { }

    /**
     * Sets the ByteSource which will be monitored through this
     * LineCountingByteSource .
     *
     * @param The new ByteSource which will be monitored.
     */
    void SetByteSource(ByteSource & bs)
    {
      m_bs = &bs;
    }

    /**
     * Sets the ByteSink that this LineCountingByteSource will use to
     * report any errors encountered during its operation.
     *
     * @param The new ByteSink used by this LineCountingByteSource for
     *        error reporting.
     */
    void SetErrorByteSink(ByteSink & bs)
    {
      m_errs = &bs;
    }

    /**
     * Sets the label of this LineCountingByteSink, which is used
     * during printing for identification.
     *
     * @param label The new string label of this LineCountingByteSink
     */
    void SetLabel(const char * label)
    {
      if (!label)
      {
        FAIL(NULL_POINTER);
      }
      m_label = label;
    }

    /**
     * Prints a formatted message to the error ByteSink held by this
     * LineCountingByteSource using \c ... style formatting. This
     * routes control to \c VMsg() directly.
     *
     * @param type The Logger Level which this formatted message will
     *             be printed at.
     *
     * @param format The formatting string used with the argument list
     *               to generate a formatted message.
     *
     * @returns \c true if \c type is less elevated than the Logger
     *          Message Level , else \c false .
     *
     * @sa VMsg()
     */
    bool Msg(Logger::Level type, const char * format, ...)
    {
      va_list ap;
      va_start(ap, format);
      VMsg(type, format, ap);
      va_end(ap);
      return type >= Logger::MESSAGE;
    }

    /**
     * Prints a formatted message to the error ByteSink held by this
     * LineCountingByteSource using a \c va_list style
     * formatting. First, the position is printed, then the kind of
     * message is printed, then the formatted message is printed.
     *
     * @param type The Logger Level which this formatted message will
     *             be printed at.
     *
     * @param format The formatting string used with the argument list
     *               to generate a formatted message.
     *
     * @param ap The argument list used with the formatting string to
     *           generate a formatted message .
     *
     * @returns \c true if \c type is less elevated than the Logger
     *          Message Level , else \c false .
     */
    bool VMsg(Logger::Level type, const char * format, va_list & ap)
    {

      PrintPosition(*m_errs);
      m_errs->Print(" ");

      switch (type)
      {
      case Logger::ERROR: m_errs->Print("error"); break;
      case Logger::WARNING: m_errs->Print("warning"); break;
      case Logger::MESSAGE: m_errs->Print("message"); break;
      default: FAIL(ILLEGAL_ARGUMENT);
      }

      m_errs->Print(": ");
      m_errs->Vprintf(format, ap);
      m_errs->Println();

      return type >= Logger::MESSAGE;
    }

    /**
     * Prints a colon separated formatted message describing this
     * LineCountingByteSink , in the format:
     *
     * <label>:<linesRead>:<bytesRead>
     *
     * @param b The ByteSink to print this formatted message to.
     */
    void PrintPosition(ByteSink & b) const
    {
      b.Printf("%s:%d:%d:", m_label, m_lineNum, m_byteNum);
    }

    /**
     * Gets the number of lines which have been read from this
     * LineCountingByteSource .
     *
     * @returns The number of lines which have been read from this
     *          LineCountingByteSource .
     */
    u32 GetLineNum() const
    {
      return m_lineNum;
    }

    /**
     * Gets the number of bytes which have been read from this
     * LineCountingByteSource .
     *
     * @returns The number of bytes which have been read from this
     *          LineCountingByteSource .
     */
    u32 GetByteNum() const
    {
      return m_byteNum;
    }

    virtual int ReadByte()
    {
      if (!m_bs)
      {
        FAIL(ILLEGAL_STATE);
      }
      // Note this sucker doesn't currently deal with Unread!  Counts
      // can be off by a byte or a line!
      s32 byte = m_bs->ReadByte();
      if (byte == '\n')
      {
        ++m_lineNum;
        m_byteNum = 0;
      }
      else
      {
        ++m_byteNum;
      }
      return byte;
    }

   private:
    ByteSource * m_bs;
    ByteSink * m_errs;
    const char * m_label;
    u32 m_lineNum;
    u32 m_byteNum;
  };
}

#endif /* LINECOUNTINGBYTESOURCE_H */
