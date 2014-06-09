/* -*- mode:C++ -*-
   ZStringByteSource.h Source bytes from a zero-terminated constant string
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
   \file ZStringByteSource.h Source bytes from a zero-terminated constant string
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef ZSTRINGBYTESOURCE_H
#define ZSTRINGBYTESOURCE_H

#include "ByteSource.h"
#include <string.h>        /* For memcpy */

namespace MFM {

  class ZStringByteSource : public ByteSource {
  public:
    ZStringByteSource(const char * input) : m_input(input), m_read(0)
    {
      if (!input)
        FAIL(NULL_POINTER);
    }

    virtual int ReadByte() {
      s32 ch = m_input[m_read];
      if (ch == 0) return -1;
      ++m_read;
      return ch;
    }

    void Reset(const char * newString = 0) {
      if (newString != 0)
        m_input = newString;
      m_read = 0;
    }

  private:
    const char * m_input;
    u32 m_read;
  };
}

#endif /* ZSTRINGBYTESOURCE_H */
