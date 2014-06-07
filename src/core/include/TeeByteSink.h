/*                                              -*- mode:C++ -*-
  TeeByteSink.h ByteSink that duplicates output onto two ByteSinks
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
  \file TeeByteSink.h ByteSink that duplicates output onto two ByteSinks
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef TEEBYTESINK_H
#define TEEBYTESINK_H

#include "ByteSink.h"

namespace MFM {

  class TeeByteSink : public ByteSink {
  public:
    TeeByteSink() : m_sink1(0), m_sink2(0)
    { }

    TeeByteSink(ByteSink & sink1, ByteSink & sink2) : m_sink1(&sink1), m_sink2(&sink2)
    { }

    ByteSink * GetSink1() const { return m_sink1; }
    ByteSink * GetSink2() const { return m_sink2; }

    ByteSink * SetSink1(ByteSink * sink) {
      ByteSink * ret = m_sink1;
      m_sink1 = sink;
      return ret;
    }

    ByteSink * SetSink2(ByteSink * sink) {
      ByteSink * ret = m_sink2;
      m_sink2 = sink;
      return ret;
    }

    virtual void WriteBytes(const u8 * data, const u32 len) {

      if (m_sink1) m_sink1->WriteBytes(data, len);

      if (m_sink2) m_sink2->WriteBytes(data, len);
    }

    virtual s32 CanWrite() {
      return
        (!m_sink1 || m_sink1->CanWrite()) &&
        (!m_sink2 || m_sink2->CanWrite());
    }

  private:
    ByteSink * m_sink1;
    ByteSink * m_sink2;
  };
}

#endif /* TEEBYTESINK_H */
