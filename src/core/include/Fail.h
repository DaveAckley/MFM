/*                                              -*- mode:C++ -*-
  fail.h Support for meaningful runtime deaths when necessary
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
  \file Fail.h Support for meaningful runtime deaths when necessary
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef FAIL_H
#define FAIL_H

#include "FailPlatformSpecific.h"  /* For FAIL and unwind_protect */

extern "C" {
  /**
     Convert a failure code into a null-terminated string for printing.
     If \a failCode is negative or greater than the number of known
     failure codes, returns "[failCode out of range]", otherwise returns
     the failure code symbol itself as a string.
  */
  const char * MFMFailCodeReason(int failCode) ;
}

#define XX(a) MFM_FAIL_CODE_REASON_##a,
enum MFMFailureCodes{
  ZERO_UNUSED = 0,
#include "FailCodes.h"
#undef XX
  MAX_FAILURE
};

#define MFM_API_ASSERT(expr,code) do { if (!(expr)) FAIL(code); } while (0)
#define MFM_API_ASSERT_NONNULL(expr) MFM_API_ASSERT(expr,NULL_POINTER)
#define MFM_API_ASSERT_ZERO(expr) MFM_API_ASSERT((expr)==0,NON_ZERO)
#define MFM_API_ASSERT_ARG(expr) MFM_API_ASSERT(expr,ILLEGAL_ARGUMENT)

#endif  /* FAIL_H */
