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
#include "CharBufferByteSource.h"
#include <string.h>        /* For memcpy */
#include <unistd.h>        /* For read */
#include <errno.h>         /* For ENOSPC e.g. */

namespace MFM {

  /**
   * A ZStringableByteSink that holds up to BUFSIZE - 2 data bytes.
   * If that limit is reached, an 'X' is appended at the end of the
   * ByteSink, and further data written is discarded.
   */
  template <u32 BUFSIZE>
  class OverflowableCharBufferByteSink : public ZStringableByteSink
  {
  public:

    /**
     * Constructs a new CharBufferByteSink representing the empty string.
     */
    OverflowableCharBufferByteSink()
    {
      Reset();
    }

    /**
     * Constructs a new OverflowableCharBufferByteSink containing a
     * copy of the given zero-terminated string.  Note the
     * initialization may cause the OverflowableCharBufferByteSink to
     * overflow; callers should test HasOverflowed() after
     * construction if they are uncertain.  FAILs if zstr is null.
     */
    OverflowableCharBufferByteSink(const char * zstr)
    {
      MFM_API_ASSERT_NONNULL(zstr);
      Reset();
      WriteBytes((const u8 *) zstr, strlen(zstr));
    }

    /**
     * Return a CharBufferByteSource that can be used to read the
     * contents of this OverflowableCharBufferByteSink, which should
     * not be changed during the lifetime of the returned
     * CharBufferByteSource.
     */
    CharBufferByteSource AsByteSource() const 
    {
      return CharBufferByteSource(GetBuffer(), GetLength());
    }

    /**
     * Attempt to read up to \c maxlen bytes from \c fd into this
     * OveflowableCharBufferByteSink .  Return 0 on EOF, negative for
     * error, otherwise the number of bytes successfully read.
     *
     * @param fd File descriptor to read from
     *
     * @param maxlen The maximum number of bytes to attempt to read
     *            from fd.  This number will be reduced as needed to
     *            avoid causing overflow.
     */
    s32 Read(s32 fd, u32 maxlen) {
      s32 room = CanWrite() - 1;
      if (m_overflowed || room <= 0) return -ENOSPC;
      if (room < (s32) maxlen) maxlen = room;
      ssize_t amt = read(fd, &m_buf[m_written], maxlen);
      if (amt < 0) return -errno;
      m_written += amt;
      m_buf[m_written] = '\0';
      return amt; // EOF or success
    }

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
      return GetLength() == str.GetLength() && !memcmp(GetBuffer(), str.GetBuffer(), GetLength());
    }

    /**
     * Gets the immutable char buffer held by this
     * OverflowableCharBufferByteSink . It is guaranteed to be null
     * terminated.
     *
     * IMPLEMENTATION NOTE: Before v3.0.0, GetZString was not const,
     * because it would explicitly null-terminate the buffer before
     * returning it.  That null termination and non-constness was how
     * GetZString differed from GetBuffer, which didn't null terminate
     * and was const.  However, doing the null terminatation in
     * GetZString caused possible data races between threads, and --
     * although those races may ultimately have been provably harmless
     * -- in the end OverflowableCharBufferByteSink was modified so
     * that it maintains the buffer as null terminated at all times.
     * Therefore, GetZString is now const and there is no fundamental
     * difference between GetZString and GetBuffer -- but it is
     * recommended to use GetZString when the caller will be counting
     * on the null termination of the buffer, and GetBuffer only when
     * the caller will not.
     *
     * @returns A null terminated string representing this
     *          OverflowableCharBufferByteSink .
     */
    virtual const char * GetZString() const
    {
      return GetBuffer();
    }

    /**
     * Gets the immutable char buffer held by this
     * OverflowableCharBufferByteSink .
     *
     * IMPLEMENTATION NOTE: Before v3.0.0, the return from this
     * function was NOT guaranteed to be null terminated, but it now
     * is.  See additional notes at GetZString .
     *
     * \sa GetZString
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
      m_buf[m_written] = '\0';
      m_overflowed = false;
    }

    /**
     * If the OverflowableCharBufferByteSink is neither empty nor
     * overflowed, and the last byte of it is equal to toChomp
     * (default value '\n'), remove that last byte, shortening the
     * content by one byte.  As an added feature, if toChomp is
     * negative, chomp any final existing non-overflowed byte.
     */
    bool Chomp(s32 toChomp = '\n')
    {
      bool chomp =
        m_written > 0 &&
        !m_overflowed &&
        (toChomp < 0 || (m_buf[m_written - 1] == (u8) toChomp));
      if (chomp) m_buf[--m_written] = '\0';
      return chomp;
    }

    bool CanChomp() { return true; }

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
     * specified string.  FAILs with OUT_OF_ROOM if the given content
     * does not fit in this OverflowableCharBufferByteSink .
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

      return *this;
    }

    /**
     * Append a copy of the current contents of this to other (or as
     * much of it as will fit).
     */
    void AppendTo(ByteSink & other) const
    {
      other.WriteBytes((const u8 *) GetBuffer(), GetLength());
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

  /**
   * An OverflowableCharBufferByteSink with a capacity of 256 bytes.
   */
  typedef OverflowableCharBufferByteSink<256 + 2> OString256;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 512 bytes.
   */
  typedef OverflowableCharBufferByteSink<512 + 2> OString512;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 1024 bytes.
   */
  typedef OverflowableCharBufferByteSink<1024 + 2> OString1024;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 2048 bytes.
   */
  typedef OverflowableCharBufferByteSink<2048 + 2> OString2048;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 4096 bytes.
   */
  typedef OverflowableCharBufferByteSink<4096 + 2> OString4096;

  /**
   * An OverflowableCharBufferByteSink with a capacity of 8192 bytes.
   */
  typedef OverflowableCharBufferByteSink<8192 + 2> OString8192;
}

#endif /* OVERFLOWABLECHARBUFFERBYTESINK_H */
