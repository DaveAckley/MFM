/*                                              -*- mode:C++ -*-
  LineTailByteSink.h ByteSink that remembers most recent lines
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
  \file LineTailByteSink.h ByteSink that remembers most recent lines
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef LINETAILBYTESINK_H
#define LINETAILBYTESINK_H

#include "OverflowableCharBufferByteSink.h"
#include <string.h>        /* For memchr */

namespace MFM {

  /**
   * A ByteSink that holds up to LINES lines each of up to
   * BYTES_PER_LINE bytes.  Excess bytes written to any given line are
   * discarded as by OverflowableCharBufferByteSink.  Once more than
   * LINES lines have been written, the oldest lines are silently
   * discarded to make room
   */
  template <u32 LINES, u32 BYTES_PER_LINE>
  class LineTailByteSink : public ByteSink {
  public:
    LineTailByteSink() : m_bytesWritten(0) {
      Reset();
    }

    virtual void WriteBytes(const u8 * data, const u32 len) {

      m_bytesWritten += len;

      const u8 * lineStart = data;
      const u8 * lineEnd;

      u32 effLen = len;

      while ((lineEnd = (const u8 *) memchr(lineStart, '\n', effLen))) {

        u32 thisLen = (u32) (lineEnd - lineStart);
        m_lines[m_nextLine].WriteBytes((const u8 *) lineStart, thisLen);
        newline();

        lineStart += thisLen + 1;
        effLen -= thisLen + 1;
      }

      m_lines[m_nextLine].WriteBytes(lineStart, effLen);
    }

    virtual s32 CanWrite() {
      return true;
    }

    u32 GetBytesWritten() const {
      return m_bytesWritten;
    }

    /**
     * How many lines are in use?  \returns From 1 up to LINES
     */
    u32 GetLines() const {
      return (LINES + m_nextLine - m_firstLine) % LINES + 1;
    }

    /**
     * Get the contents of \a whichLine as a zero-terminated string.
     * If there is (currently) no line \a whichLine, return NULL
     */
    const char * GetZString(u32 whichLine) {
      if (whichLine >= GetLines()) return 0;

      u32 index = (m_firstLine + whichLine) % LINES;
      return m_lines[index].GetZString();
    }

    /**
     * Discard the oldest \a lines lines, if there are that many,
     * otherwise just do a Reset.
     */
    void Trim(u32 lines) {
      if (lines >= GetLines()) Reset();
      else m_firstLine = (m_firstLine + lines) % LINES;
    }

    void Reset() {
      m_firstLine = m_nextLine = 0;
      m_lines[m_nextLine].Reset();
    }

  private:
    void newline() {
      m_nextLine = (m_nextLine + 1) % LINES;
      m_lines[m_nextLine].Reset();

      if (m_nextLine == m_firstLine)
        m_firstLine = (m_firstLine + 1) % LINES;
    }
    OverflowableCharBufferByteSink<BYTES_PER_LINE> m_lines[LINES];
    u32 m_bytesWritten;
    u32 m_firstLine;
    u32 m_nextLine;
  };
}

#endif /* LINETAILBYTESINK_H */
