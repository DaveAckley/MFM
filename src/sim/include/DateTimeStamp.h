/*                                              -*- mode:C++ -*-
  DateTimeStamp.h A serializer for date/times
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
  \file DateTimeStamp.h A serializer for date/times
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef DATETIMESTAMP_H
#define DATETIMESTAMP_H

#include <stdlib.h>
#include "ByteSerializable.h"
#include "itype.h"
#include "Utils.h"

namespace MFM
{
  class DateTimeStamp : public ByteSerializable {
    u64 m_lastDateTime;
    u32 m_sequence;
  public:
    DateTimeStamp() : m_lastDateTime(0), m_sequence(0) { }
    void Reset() { m_lastDateTime = 0; }
    virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      m_lastDateTime = Utils::GetDateTimeNow();
      byteSink.Print(m_lastDateTime);
      if (argument == 0)
      {
        byteSink.Print("-");
        byteSink.Print(m_sequence, Format::LEX32);
        byteSink.Print(": ");
      }
      ++m_sequence;
      return SUCCESS;
    }

    virtual Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      return UNSUPPORTED;
    }

  };
}

#endif /* DATETIMESTAMP_H */
