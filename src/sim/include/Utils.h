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
#include "ByteSink.h"

namespace MFM
{
  /**
     Utility functions at the sim/ level of the code.  Unlike the
     functions in Util.h , functions here can use the awesome
     firepower of this fully-armed and operational Linux system,
     including things like dates and file systems.
   */
  namespace Utils {

    /**
       Return the current time, formatted as described under
       GetDateTime().

       \sa GetDateTime
     */
    u64 GetDateTimeNow() ;

    /**
       Return the given time, formatted so that if the result is
       printed in decimal, it will produce a (currently) length 14
       string like 20150611132732, structured as YYYYMMDDHHMMSS.
     */
    u64 GetDateTime(time_t t) ;

    /**
       Given a datetime formatted as by GetDateTime(), return the
       YYYMMDD portion of it, like 20150611 from 20150611132732
    */
    u32 GetDateFromDateTime(u64 datetime) ;

    /**
       Given a datetime formatted as by GetDateTime(), return the
       HHMMSS portion of it, like 132732 from 20150611132732
    */
    u32 GetTimeFromDateTime(u64 datetime) ;

    /**
       Try to develop a path based on the directory of the running
       binary plus a possible \c pathSuffixOrNull.  Returns \c false
       if the path cannot be constructed for any reason, including:
       path component or overall length too big, permission failure,
       too many symbolic links, or the INDICATED PATH DOES NOT EXIST.

       If a path is successfully resolved, returns \c true and writes
       the path to \c result.  NOTE THAT IT IS UP TO THE CALLER to
       reset \c result before, if needed, and to check for overflow of
       \c result after, if that may be an issue.
    */
    bool ResolveProgramRelativePath(ByteSink& result, const char * pathSuffixOrNull) ;

    /**
       Look in standard places for relativePath.  Return true if a
       readable file was found, and fills result with an absolute path
       (up to the given length) that was openable for reading at the
       time it was checked.  If such a readable file is not found
       anywhere, set result to an empty string and return false.
       Fails with ILLEGAL_ARGUMENT if relativePath or result is NULL
       or if length is zero; will silently truncate paths (and fail to
       find a file, or worse, possibly find the wrong file) if length
       is too short (but if result supports the HasOverflowed()
       method, that can be checked after the call.)
    */
    bool GetReadableResourceFile(const char * relativePath, ByteSink & result) ;

    /**
       Copy path to buffer, while expanding a leading '~' in path by
       replacing it with getenv("HOME") if defined.  Fails if path is
       NULL; if the (possibly expanded) path did not fit in buffer,
       buffer.HasOverflowed() will be true after the call (assuming
       buffer supports that method).
    */
    void NormalizePath(const char * path, ByteSink & buffer) ;

    /**
       Return null if path can currently be successfully opened for
       reading, or return a string describing the error if it cannot.

       The returned string is static and (of course) should not be
       freed.

       \sa ReadablePath(const OverflowableCharBufferByteSink<BUFSIZE> & )
     */
    const char * ReadablePath(const char * path) ;

  }
}

#endif /* UTILS_H */
