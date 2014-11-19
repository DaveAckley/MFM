/* -*- mode:C++ -*-
   CharBufferByteSource.h Source bytes from a fixed char buffer
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
   \file CharBufferByteSource.h Source bytes from a fixed char buffer
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef CHARBUFFERBYTESOURCE_H
#define CHARBUFFERBYTESOURCE_H

#include "ByteSource.h"

namespace MFM
{
  /**
   * A ByteSource backed by a char pointer.
   */
  class CharBufferByteSource : public ByteSource
  {
  public:

    /**
     * Constructs a new CharBufferByteSource backed by a provided char
     * pointer. The length of readable bytes by the provided char
     * pointer must also be provided.
     *
     * @param input The char* to back this CharBufferByteSource
     *              by. This must not be NULL, else this constructor
     *              will FAIL with NULL_POINTER .
     *
     * @param length The number of readable bytes which may be read
     *               from \c input .
     */
    CharBufferByteSource(const char * input, u32 length)
      : m_input(input),
	m_length(length),
	m_read(0)
    {
      if (!input)
      {
        FAIL(NULL_POINTER);
      }
    }

    /**
     * Reads the next byte from the front of this CharBufferByteSource
     * and advances the pointer.
     *
     * @returns the next byte from the front of this CharBufferByteSource .
     */
    virtual int ReadByte()
    {
      if (m_read >= m_length)
      {
	return -1;
      }
      return (u8) m_input[m_read++]; // cast for non-negative result
    }

    /**
     * Assigns a new char pointer to this CharBufferByteSource. Used
     * to reconstruct this CharBufferByteSource as needed.
     *
     * @param newBuffer The new char pointer which this
     *                  CharBufferByteSource will point to. This must
     *                  not be NULL, else will FAIL with NULL_POINTER .
     *
     * @param bufferLength The number of readable bytes which may be read
     *                     from \c newBuffer .
     */
    void ChangeBuffer(const char * newBuffer, u32 bufferLength)
    {
      if (!newBuffer)
      {
        FAIL(NULL_POINTER);
      }
      m_input = newBuffer;
      m_length = bufferLength;

      Reset();
    }

    /**
     * Effectively clears this CharBufferByteSource, making all bytes
     * since either construction or \c ChangeBuffer() available again
     * from the beginning of the buffer.
     */
    void Reset()
    {
      m_read = 0;
    }

  private:

    /**
     * The char pointer of which to read bytes from upon \c ReadByte()
     * . This is guaranteed to not be NULL .
     */
    const char * m_input;

    /**
     * The number of bytes which may be read from this
     * CharBufferByteSource .
     */
    u32 m_length;

    /**
     * Thenumber of bytes which have already been read from this
     * CharBufferByteSource.
     */
    u32 m_read;
  };
}

#endif /* CHARBUFFERBYTESOURCE_H */
