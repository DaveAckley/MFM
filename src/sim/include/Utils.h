#ifndef UTILS_H     /* -*- C++ -*- */
#define UTILS_H

#include <stdlib.h>
#include "itype.h"

namespace MFM
{
  namespace Utils {
    u64 GetDateTimeNow() ;

    u64 GetDateTime(time_t t) ;

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
