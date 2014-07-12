/*                                              -*- mode:C++ -*-
  CharBufferByteSink.h Character based ByteSink
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
  \file CharBufferByteSink.h Character based ByteSink
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CHARBUFFERBYTESINK_H
#define CHARBUFFERBYTESINK_H

#include "ByteSink.h"
#include <string.h>        /* For memcpy */

namespace MFM
{
  /**
   * A template class for a ByteSink into a fixed-size char buffer.
   * If the fixed-size buffer overflows,
   * CharBufferByteSink::WriteBytes will FAIL with OUT_OF_ROOM.  For
   * an alternative possibly more useful approach, see \ref
   * OverflowableCharBufferByteSink.
   */
  template <int BUFSIZE>
  class CharBufferByteSink : public ByteSink
  {
  public:

    CharBufferByteSink() :
      m_written(0)
      { }

    /**
     * Writes a series of bytes to this CharBufferByteSink. This will
     * FAIL with OUT_OF_ROOM if therre is not enough room for the
     * bytes to be written. One can check the number of bytes which
     * may be written through the \c CanWrite() function.
     *
     * @param data A pointer to the bytes wished to be written to this
     *             CharBufferByteSink.
     *
     * @param len The number of bytes to write to this
     *            CharBufferByteSink.
     */
    virtual void WriteBytes(const u8 * data, const u32 len)
    {
      if (m_written + len < BUFSIZE-1)
      {
        memcpy(&m_buf[m_written], data, len);
        m_written += len;
        return;
      }
      FAIL(OUT_OF_ROOM);
    }

    /**
     * Gets the number of bytes which may be written to this
     * CharBufferByteSink before it calls FAIL(OUT_OF_ROOM) .
     *
     * @returns The number of bytes which may be written to this
     *          CharBufferByteSink .
     */
    virtual s32 CanWrite()
    {
      return BUFSIZE - m_written - 1;
    }

    /**
     * Checks to see if the underlying buffer of this
     * CharBufferByteSink contains the same characters of another
     * null-terminated string.
     *
     * @param str A null-terminated string of characters which may or
     *            may not equal the string held by this
     *            CharBufferByteSink .
     *
     * @returns \c true if this CharBufferByteSink and \c str
     *          represent the same string, else \c false.
     */
    bool Equals(const char * str)
    {
      return strcmp(GetZString(), str)==0;
    }

    /**
     * Retreives the underlying buffer of this CharBufferByteSink,
     * usable as a \c const \c char* . The returned \c char* is
     * guaranteed to be null terminated.
     *
     * @returns The underlying buffer of this CharBufferByteSink,
     * usable as a \c const \c char* .
     */
    const char * GetZString()
    {
      m_buf[m_written] = '\0';
      return (const char *) m_buf;
    }

    /**
     * Gets the length of this CharBufferByteSink, in bytes.
     *
     * @returns The length of this CharBufferByteSink, in bytes.
     */
    u32 GetLength() const
    {
      return m_written;
    }

    /**
     * Gets the maximum number of bytes which this CharBufferByteSink
     * may hold.
     *
     * @returns The maximum number of bytes which this CharBufferByteSink
     *          may hold.
     */
    u32 GetCapacity() const
    {
      return BUFSIZE;
    }

    /**
     * Effectively clears this CharBufferByteSink, resulting in it
     * being logically equivalent to the empty string.
     */
    void Reset()
    {
      m_written = 0;
    }

  private:
    /**
     * The underlying buffer used to represent the string held by this
     * CharBufferByteSink.
     */
    u8 m_buf[BUFSIZE];

    /**
     * The number of bytes that this CharBufferByteSink is currently
     * holding.
     */
    u32 m_written;
  };
}

#endif /* CHARBUFFERBYTESINK_H */
