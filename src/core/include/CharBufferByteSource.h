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

namespace MFM {

  class CharBufferByteSource : public ByteSource {
  public:
    CharBufferByteSource(const char * input, u32 length)
      : m_input(input), m_length(length), m_read(0)
    {
      if (!input)
        FAIL(NULL_POINTER);
    }

    virtual int ReadByte() {
      if (m_read >= m_length) return -1;
      return m_input[m_read++];
    }

    void ChangeBuffer(const char * newBuffer, u32 bufferLength) {
      if (!newBuffer)
        FAIL(NULL_POINTER);
      m_input = newBuffer;
      m_length = bufferLength;

      Reset();
    }

    void Reset() {
      m_read = 0;
    }

  private:
    const char * m_input;
    u32 m_length;
    u32 m_read;
  };
}

#endif /* CHARBUFFERBYTESOURCE_H */
