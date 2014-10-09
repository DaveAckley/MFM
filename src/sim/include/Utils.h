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
#include <time.h>  /* for nanosleep */
#include "itype.h"

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
       anywhere, set result to a null string and return false.  Fails
       with ILLEGAL_ARGUMENT if length is zero; will silently truncate
       paths (and fail to find a file, or worse, possibly find the wrong
       file) if length is too short.
    */
    bool GetReadableResourceFile(const char * relativePath, char * result, u32 length) ;

  }
}

#endif /* UTILS_H */
