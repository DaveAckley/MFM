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
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef LINETAILBYTESINK_H
#define LINETAILBYTESINK_H

#include "OverflowableCharBufferByteSink.h"
#include <string.h>        /* For memchr */

namespace MFM
{

  /**
   * A ByteSink that holds up to LINES lines each of up to
   * BYTES_PER_LINE bytes.  Excess bytes written to any given line are
   * discarded as by OverflowableCharBufferByteSink.  Once more than
   * LINES lines have been written, the oldest lines are silently
   * discarded to make room.
   */
  template <u32 LINES, u32 BYTES_PER_LINE>
  class LineTailByteSink : public ByteSink
  {
  public:

    /**
     * Constructs a new LineTailByteSink and calls \c Reset() on it.
     */
    LineTailByteSink() : m_bytesWritten(0)
    {
      Reset();
    }

    /**
     * Writes a series of bytes to this LineTailByteSink, keeping
     * track of the lines encountered and discarding bytes encountered
     * past the length of a line that is too long.
     *
     * @param data A pointer to the bytes to write to this
     *             LineTailByteSink .
     *
     * @param len The number of bytes wished to write to this
     *            LineTailByteSink . All len bytes are written before
     *            this method returns .
     */
    virtual void WriteBytes(const u8 * data, const u32 len)
    {
      m_bytesWritten += len;

      const u8 * lineStart = data;
      const u8 * lineEnd;

      u32 effLen = len;

      while ((lineEnd = (const u8 *) memchr(lineStart, '\n', effLen)))
      {

        u32 thisLen = (u32) (lineEnd - lineStart);
        m_lines[m_nextLine].WriteBytes((const u8 *) lineStart, thisLen);
        newline();

        lineStart += thisLen + 1;
        effLen -= thisLen + 1;
      }

      m_lines[m_nextLine].WriteBytes(lineStart, effLen);
    }

    /**
     * Checks to see if bytes may be written to this LineTailByteSink
     * . Since this is backed by an OverflowableCharBufferByteSink
     * array, this may always be written to.
     *
     * @returns \c true .
     */
    virtual s32 CanWrite()
    {
      return true;
    }

    /**
     * Gets the number of bytes that have been written to this
     * LineTailByteSink .
     *
     * @returns The number of bytes that have been written to this
     *          LineTailByteSink .
     */
    u32 GetBytesWritten() const
    {
      return m_bytesWritten;
    }

    /**
     * Gets the number of lines that have been encountered during the
     * lifetime of this LineTailByteSink .
     *
     * @returns The number of lines that have been encountered by this
     *          LineTailByteSink, from 1 up to LINES .
     */
    u32 GetLines() const
    {
      return (LINES + m_nextLine - m_firstLine) % LINES + 1;
    }

    /**
     * Get the contents of \a whichLine as a zero-terminated string.
     * If there is (currently) no line \a whichLine, return NULL
     */

    /**
     * Gets the contents of a line held by this LineTailByteSink ,
     * where the line at index \c 0 is the oldest line .
     *
     * @param whichLine the index of the line to get from this
     *                  LineTailByteSink .
     *
     * @returns A pointer to the zero-terminated line at \c whichLine
     *          index in this LineTailByteSink .
     */
    const char * GetZString(u32 whichLine)
    {
      if (whichLine >= GetLines())
      {
	return 0;
      }

      u32 index = (m_firstLine + whichLine) % LINES;
      return m_lines[index].GetZString();
    }

    /**
     * Discard the oldest \a lines lines, if there are that many,
     * otherwise just do a Reset.
     */

    /**
     * Discards a number of old lines inside this LineTailByteSink .
     *
     * @param lines The number of lines to discard from this
     *              LineTailByteSink . If this is more than the number
     *              of lines held , will call \c Reset() .
     */
    void Trim(u32 lines)
    {
      if (lines >= GetLines())
      {
	Reset();
      }
      else m_firstLine = (m_firstLine + lines) % LINES;
    }

    /**
     * Clears this LineTailByteSink , as if it is newly constructed .
     */
    void Reset()
    {
      m_firstLine = m_nextLine = 0;
      m_lines[m_nextLine].Reset();
    }

  private:
    void newline()
    {
      m_nextLine = (m_nextLine + 1) % LINES;
      m_lines[m_nextLine].Reset();

      if (m_nextLine == m_firstLine)
      {
        m_firstLine = (m_firstLine + 1) % LINES;
      }
    }

    OverflowableCharBufferByteSink<BYTES_PER_LINE> m_lines[LINES];
    u32 m_bytesWritten;
    u32 m_firstLine;
    u32 m_nextLine;
  };
}

#endif /* LINETAILBYTESINK_H */
