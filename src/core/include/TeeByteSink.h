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
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef TEEBYTESINK_H
#define TEEBYTESINK_H

#include "ByteSink.h"
#include "Util.h"     /* For MIN */

namespace MFM
{

  /**
   * A ByteSink that wraps two other ByteSinks, routing all bytes sent
   * to it on to both underlying ByteSinks.
   */
  class TeeByteSink : public ByteSink
  {
  public:

    /**
     * Creates a new TeeByteSink which contains two NULL pointers as
     * its underlying ByteSinks. These need to be set before this
     * ByteSink can become useful.
     *
     * @sa SetSink1
     * @sa SetSink2
     */
    TeeByteSink() :
      m_sink1(0),
      m_sink2(0)
    { }

    /**
     * Creates a new TeeByteSink which points to the specifiend
     * ByteSinks. Either or both of these ByteSinks may be NULL,
     * which will not be written to.
     *
     * @param sink1 The first ByteSink that this TeeByteSink will
     *              initially begin writing to.
     *
     * @param sink1 The second ByteSink that this TeeByteSink will
     *              initially begin writing to.
     */
    TeeByteSink(ByteSink & sink1, ByteSink & sink2) :
      m_sink1(&sink1),
      m_sink2(&sink2)
    { }

    /**
     * Gets a pointer to the first ByteSink that this TeeByteSink is
     * currently configured to write to.
     *
     * @returns A pointer to the first ByteSink that this TeeByteSink
     *          is currently configured to write to.
     */
    ByteSink * GetSink1() const
    {
      return m_sink1;
    }

    /**
     * Gets a pointer to the second ByteSink that this TeeByteSink is
     * currently configured to write to.
     *
     * @returns A pointer to the second ByteSink that this TeeByteSink
     *          is currently configured to write to.
     */
    ByteSink * GetSink2() const
    {
      return m_sink2;
    }

    /**
     * Sets the first ByteSink that this TeeByteSink will be
     * configured to write to.
     *
     * @param sink The first ByteSink that this TeeByteSink will be
     *             configured to write to after calling this method.
     *
     * @returns A pointer to the first ByteSink that this TeeByteSink
     *          was configured to write to before calling this method.
     */
    ByteSink * SetSink1(ByteSink * sink)
    {
      ByteSink * ret = m_sink1;
      m_sink1 = sink;
      return ret;
    }

    /**
     * Sets the second ByteSink that this TeeByteSink will be
     * configured to write to.
     *
     * @param sink The second ByteSink that this TeeByteSink will be
     *             configured to write to after calling this method.
     *
     * @returns A pointer to the second ByteSink that this TeeByteSink
     *          was configured to write to before calling this method.
     */
    ByteSink * SetSink2(ByteSink * sink)
    {
      ByteSink * ret = m_sink2;
      m_sink2 = sink;
      return ret;
    }

    /**
     * Writes a series of bytes to the ByteSinks held by this
     * TeeByteSink. If either of these held ByteSinks happen to be
     * NULL, writing to that ByteSink is ignored.
     *
     * @param data A pointer to the bytes that will be written to the
     *             held ByteSinks.
     *
     * @param len The number of bytes which will be written to the
     *            held ByteSinks.
     */
    virtual void WriteBytes(const u8 * data, const u32 len)
    {
      if (m_sink1)
      {
	m_sink1->WriteBytes(data, len);
      }

      if (m_sink2)
      {
	m_sink2->WriteBytes(data, len);
      }
    }

    /**
     * Gets the maximum number of bytes which may be written to both
     * of the ByteSinks held by this TeeByteSink.
     *
     * @returns The maximum number of bytes which may be written to
     *          both of the ByteSinks held by this TeeByteSink.
     */
    virtual s32 CanWrite()
    {
      return MIN<s32>(m_sink1 ? m_sink1->CanWrite() : S32_MAX,
                      m_sink2 ? m_sink2->CanWrite() : S32_MAX);
    }

  private:
    ByteSink * m_sink1;
    ByteSink * m_sink2;
  };
}

#endif /* TEEBYTESINK_H */
