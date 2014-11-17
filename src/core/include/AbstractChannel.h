/*                                              -*- mode:C++ -*-
  AbstractChannel.h Intertile communications interface
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
  \file AbstractChannel.h Intertile communications interface
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTCHANNEL_H
#define ABSTRACTCHANNEL_H

#include "itype.h"

namespace MFM
{
  /**
   * An AbstractChannel mediates bidirectional communications between
   * two Tiles, which are denoted A and B.
   */
  class AbstractChannel
  {

  public:
    /**
     * Destroys this AbstractChannel.
     */
    virtual ~AbstractChannel()
    { }

    /**
     * Return a lower bound on the number of bytes that A (if byA) or
     * B (if not byA) can currently write into the channel.  In other
     * words, if CanWrite returns X, than a following Write specifying
     * length <= X will return length.
     */
    virtual u32 CanWrite(bool byA) = 0;

    /**
     * Write up to length bytes to A's (if byA) or B's (if not byA)
     * output channel.  Return the number of bytes actually (queued to
     * be) written, from 0 up to length.
     */
    virtual u32 Write(bool byA, const u8 * data, u32 length) = 0;

    /**
     * Return a lower bound on the number of bytes that A (if byA) or
     * B (if not byA) can currently read from the channel.  In other
     * words, if CanRead returns X, than a following Read specifying
     * length <= X will return length.
     */
    virtual u32 CanRead(bool byA) = 0;

    /**
     * Read up to length bytes from A's (if byA) or B's (if not byA)
     * input channel.  Return the number of bytes actually read, from
     * 0 up to length.
     */
    virtual u32 Read(bool byA, u8 * data, u32 length) = 0;

  };
}

#endif /* ABSTRACTCHANNEL_H */
