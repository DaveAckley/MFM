/*                                              -*- mode:C++ -*-
  ByteSerializable.h Interface for objects that print and read themselves
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
  \file ByteSerializable.h Interface for objects that print and read themselves
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef BYTESERIALIZABLE_H
#define BYTESERIALIZABLE_H

#include "itype.h"
#include "Fail.h"
#include "Format.h"
#include "ByteSink.h"
#include "ByteSource.h"
#include <stdarg.h>    /* For ... */

namespace MFM
{
  /**
     ByteSerializable is an interface for an object that can either
     print itself to a ByteSink, read itself from a ByteSource, or
     both.  By default it can do neither; if it can do both, it is
     strongly expected that the two operations invert each other.
   */
  class ByteSerializable
  {
   public:

    enum Result
    {
      UNSUPPORTED = -1,
      FAILURE = 0,
      SUCCESS = 1
    };
    /**
       Modify this object by reading a representation of a
       ByteSerializable from the given \a byteSource, with its details
       possibly modified by \a argument.  The meaning of \a argument
       is unspecified except that 0 should be interpreted as a request
       for default, 'no modification', interpretation. \returns
       UNSUPPORTED if this object doesn't support reading itself, or
       SUCCESS if the reading was successful and FAIL if there was any
       problem.
     */
    virtual Result ReadFrom(ByteSource & byteSource, s32 argument = 0) = 0;
    /*
    {
      return UNSUPPORTED;
    }
    */

    /**
       Print a representation of this ByteSerializble to the given \a
       byteSink, with its details possibly modified by \a argument.
       The meaning of \a argument is unspecified except that 0 should
       be interpreted as a request for default, 'no modification',
       formatting. \returns UNSUPPORTED if this object doesn't support
       printing itself, or SUCCESS if the printing was successful and
       FAIL if there was any problem.
     */
    virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0) = 0;
    /*
    {
      return UNSUPPORTED;
      }*/

    virtual ~ByteSerializable()
    { }
  };

  class NullByteSerializable : public ByteSerializable
  {
    virtual Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      return SUCCESS;
    }

    virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      return SUCCESS;
    }
  };

  extern NullByteSerializable NullSerializable;

}

#endif /* BYTESERIALIZABLE_H */
