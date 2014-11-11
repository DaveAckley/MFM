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
   * An AbstractChannel mediates locking and bidirectional
   * communications between two Tiles, which are denoted A and B.
   */
  class AbstractChannel
  {

  public:
    enum State
    {
      CHANNEL_UNOWNED,          // Channel is unowned
      CHANNEL_A_OWNER,          // A owns the channel
      CHANNEL_B_OWNER           // B owns the channel
    };

    /**
     * Destroys this AbstractChannel.
     */
    virtual ~AbstractChannel()
    { }

    /**
     * Get the current channel state.  The result is only advisory
     * (i.e., it may have changed by the time caller looks at it)
     * unless the caller owns the channel.
     */
    virtual State GetChannelState() = 0;

    /**
     * Atomically check and possibly update the channel state as
     * follows: If the channel state is currently CHANNEL_UNOWNED,
     * change it to CHANNEL_A_OWNER (if byA is true) or
     * CHANNEL_B_OWNER (if byA is false) and return true.  Otherwise
     * do not change the channel state and return false.
     */
    virtual bool TryLock(bool byA) = 0;

    /**
     * Atomically check and possibly update the channel state as
     * follows: If the channel state is currently CHANNEL_A_OWNER and
     * byA is true, or the channel state is CHANNEL_B_OWNER and byA is
     * false, change the channel state to CHANNEL_UNOWNED and return
     * true.  Otherwise do not change the channel state and return
     * false.
     */
    virtual bool Unlock(bool byA) = 0;

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
