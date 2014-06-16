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
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CHARBUFFERBYTESINK_H
#define CHARBUFFERBYTESINK_H

#include "ByteSink.h"
#include <string.h>        /* For memcpy */

namespace MFM {

  /**
   * A template class for a ByteSink into a fixed-size char buffer.
   * If the fixed-size buffer overflows,
   * CharBufferByteSink::WriteBytes will FAIL with OUT_OF_ROOM.  For
   * an alternative possibly more useful approach, see \ref
   * OverflowableCharBufferByteSink.
   */
  template <int BUFSIZE>
  class CharBufferByteSink : public ByteSink {
  public:
    CharBufferByteSink() : m_written(0) { }
    virtual void WriteBytes(const u8 * data, const u32 len) {
      if (m_written + len < BUFSIZE-1) {
        memcpy(&m_buf[m_written], data, len);
        m_written += len;
        return;
      }
      FAIL(OUT_OF_ROOM);
    }

    virtual s32 CanWrite() {
      return BUFSIZE - m_written - 1;
    }

    bool Equals(const char * str) {
      return strcmp(GetZString(), str)==0;
    }
    const char * GetZString() {
      m_buf[m_written] = '\0';
      return (const char *) m_buf;
    }

    u32 GetLength() const {
      return m_written;
    }

    u32 GetCapacity() const {
      return BUFSIZE;
    }

    void Reset() {
      m_written = 0;
    }

  private:
    u8 m_buf[BUFSIZE];
    u32 m_written;
  };
}

#endif /* CHARBUFFERBYTESINK_H */
