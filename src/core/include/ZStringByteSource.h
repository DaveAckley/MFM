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

#include "CharBufferByteSource.h"
#include <string.h>        /* For strlen */

namespace MFM {

  /**
     Source bytes from a zero-terminated constant string.
   */
  class ZStringByteSource : public CharBufferByteSource {
  private: typedef CharBufferByteSource Super;

    static u32 Strlen(const char * ptr) {
      if (!ptr)
        FAIL(NULL_POINTER);
      return strlen(ptr);
    }

  public:
    ZStringByteSource(const char * input) : Super(input, Strlen(input))
    { }

    void Reset(const char * newString = 0) {
      if (newString != 0)
        ChangeBuffer(newString, Strlen(newString));
      else
        Super::Reset();
    }

  };
}

#endif /* ZSTRINGBYTESOURCE_H */
