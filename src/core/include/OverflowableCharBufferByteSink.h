/*                                              -*- mode:C++ -*-
  OverflowableCharBufferByteSink.h Overflowable character stream
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
  \file OverflowableCharBufferByteSink.h Overflowable character stream
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef OVERFLOWABLECHARBUFFERBYTESINK_H
#define OVERFLOWABLECHARBUFFERBYTESINK_H

#include "ByteSink.h"
#include <string.h>        /* For memcpy */

namespace MFM {

  /**
   * A ByteSink that holds up to BUFSIZE - 2 data bytes.  If that
   * limit is reached, an 'X' is appended at the end of the ByteSink,
   * and further data written is discarded.
   */
  template <u32 BUFSIZE>
  class OverflowableCharBufferByteSink : public ByteSink
  {
  public:

    /**
     * Constructs a new CharBufferByteSink representing the empty string.
     */
    OverflowableCharBufferByteSink() :
      m_written(0),
      m_overflowed(false)
      { }

    /**
     * Writes a series of bytes to this OveflowableCharBufferByteSink
     * . If overflow occurs, will append an 'X' to the end of this
     * internal string.
     *
     * @param bytes A pointer to the bytes which will be written to this
     *              OverflowableCharBufferByteSink .
     *
     * @param len The number of bytes which will be attempted to be
     *            written to this OverflowableCharBufferByteSink .
     */
    virtual void WriteBytes(const u8 * data, const u32 len)
    {
      if (m_overflowed)
      {
        return;
      }

      u32 effLen = len;
      if (m_written + effLen > BUFSIZE - 2)
      {
        effLen = BUFSIZE - 2 - m_written;
        m_overflowed = true;
      }

      memcpy(&m_buf[m_written], data, effLen);
      m_written += effLen;

      if (m_overflowed)
      {
        m_buf[m_written++] = 'X';
      }

      m_buf[m_written] = '\0'; // HELGRIND
    }

    /**
     * Gets the number of bytes that can be written to this
     * OverflowableCharBufferByteSink .
     *
     * @returns The number of bytes that can be written to this
     * OverflowableCharBufferByteSink .
     */
    virtual s32 CanWrite()
    {
      return BUFSIZE - m_written - 1;
    }

    /**
     * Checks to see if the string represented by this
     * OverflowableCharBufferByteSink is equal to another specified
     * string.
     *
     * @param str The string to check for equality against this
     *            OverflowableCharBufferByteSink .
     *
     * @returns \c true if the string represented by this
     *          OverflowableCharBufferByteSink is equal to the string
     *          represented by \c str , else \c false .
     */
    bool Equals(const char * str) const
    {
      return GetLength() == strlen(str) && !memcmp(m_buf, str, GetLength());
    }

    /**
     * Checks to see if the string represented by this
     * OverflowableCharBufferByteSink is equal to another specified
     * OverflowableCharBufferByteSink.
     *
     * @param str The OverflowableCharBufferByteSink to check for
     *            equality against this OverflowableCharBufferByteSink
     *
     *
     * @returns \c true if the string represented by this
     *          OverflowableCharBufferByteSink is equal to the string
     *          represented by \c str , else \c false .
     */
    template <u32 OTHER_SIZE>
    bool Equals(const OverflowableCharBufferByteSink<OTHER_SIZE> & str) const
    {
      return GetLength() == str.GetLength() && !memcmp(m_buf, str.m_buf, GetLength());
    }

    /**
     * Gets the immutable char buffer held by this
     * OverflowableCharBufferByteSink . It is guaranteed to be null
     * terminated.
     *
     * @returns A null terminated string representing this
     *          OverflowableCharBufferByteSink .
     */
    const char * GetZString()
    {
      // HELGRIND      m_buf[m_written] = '\0';
      return GetBuffer();
    }

    /**
     * Gets the immutable char buffer held by this
     * OverflowableCharBufferByteSink . This is NOT guaranteed to be
     * null terminated!
     *
     * @returns A pointer to the string representing this
     *          OverflowableCharBufferByteSink which may not be null
     *          terminated.
     */
    const char * GetBuffer() const
    {
      return (const char *) m_buf;
    }

    /**
     * Gets the length of the string represented by this
     * OverflowableCharBufferByteSink in bytes .
     *
     * @returns The length of the string represented by this
     *          OverflowableCharBufferByteSink in bytes .
     */
    u32 GetLength() const
    {
      return m_written;
    }

    /**
     * Gets the total number of bytes that may be written to this
     * OverflowableCharBufferByteSink before it overflows.
     *
     * @returns The total number of bytes that may be written to this
     *          OverflowableCharBufferByteSink before it overflows.
     */
    u32 GetCapacity() const
    {
      return BUFSIZE;
    }

    /**
     * Resets the internal state of this
     * OverflowableCharBufferByteSink , making it equivalent to the
     * empty string.
     */
    void Reset()
    {
      m_written = 0;
      m_overflowed = false;
    }

    /**
     * Checks to see whether or not this
     * OverflowableCharBufferByteSink has overflowed, i.e. had more
     * bytes written to it than it can hold.
     *
     * @returns \c true if this OverflowableCharBufferByteSink has
     *          overflowed before, else \c false .
     */
    bool HasOverflowed() const
    {
      return m_overflowed;
    }

    /**
     * An operator overload which copies the bytes held at a specified
     * null terminated string to this OverflowableCharBufferByteSink
     * . After this is called, and if there is enough room in this
     * OverflowableCharBufferByteSink , it will represent the
     * specified string.
     *
     * @param zstr The null terminated string which is wished to be
     *             represented by this OverflowableCharBufferByteSink .
     */
    OverflowableCharBufferByteSink<BUFSIZE>& operator=(const char* zstr)
    {
      if(strlen(zstr) >= GetCapacity())
      {
        FAIL(OUT_OF_ROOM);
      }

      Reset();
      Print(zstr);
      GetZString(); // null terminate

      return *this;
    }

  private:
    u32 m_written;
    bool m_overflowed;
    u8 m_buf[BUFSIZE];
  };

  /**
   * An OverflowableCharBufferByteSink with a capacity of 16 bytes.
   */
  typedef OverflowableCharBufferByteSink<16 + 2> OString16;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 32 bytes.
   */
  typedef OverflowableCharBufferByteSink<32 + 2> OString32;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 64 bytes.
   */
  typedef OverflowableCharBufferByteSink<64 + 2> OString64;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 128 bytes.
   */
  typedef OverflowableCharBufferByteSink<128 + 2> OString128;
}

#endif /* OVERFLOWABLECHARBUFFERBYTESINK_H */
