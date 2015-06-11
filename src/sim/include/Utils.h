/*                                              -*- mode:C++ -*-
  Utils.h Globally accessible extension methods
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
  \file Utils.h Globally accessible extension methods
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <time.h>   /* for nanosleep */
#include "itype.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM
{
  namespace Utils {

    u64 GetDateTimeNow() ;

    u64 GetDateTime(time_t t) ;

    u32 GetDateFromDateTime(u64 datetime) ;

    u32 GetTimeFromDateTime(u64 datetime) ;

    /**
       Look in standard places for relativePath.  Return true if a
       readable file was found, and fills result with an absolute path
       (up to the given length) that was openable for reading at the
       time it was checked.  If such a readable file is not found
       anywhere, set result to an empty string and return false.  Fails
       with ILLEGAL_ARGUMENT if relativePath or result is NULL or if
       length is zero; will silently truncate paths (and fail to find
       a file, or worse, possibly find the wrong file) if length is
       too short.
    */
    bool GetReadableResourceFile(const char * relativePath, char * result, u32 length) ;

    /**
       Copy path to buffer, an OString of some size, while expanding a
       leading '~' in path by replacing it with getenv("HOME") if
       defined, in buffer.  Fails with ILLEGAL_ARGUMENT if path is
       NULL; if the (possibly expanded) path did not fit in buffer,
       buffer.HasOverflowed() will be true after the call.
    */
    template<u32 BUFSIZE>
    void NormalizePath(const char * path, OverflowableCharBufferByteSink<BUFSIZE> & buffer)
    {
      buffer = path;
      if (path[0] == '~') {
        const char * home = getenv("HOME");
        if (home) {
          buffer.Reset();
          buffer.Printf("%s%s", home, path + 1);
        }
      }
    }

    const char * ReadablePath(const char * path) ;

    template<u32 BUFSIZE>
    const char * ReadablePath(const OverflowableCharBufferByteSink<BUFSIZE> & path)
    {
      return ReadablePath(path.GetZString());
    }
  }
}

#endif /* UTILS_H */
