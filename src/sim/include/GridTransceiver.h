/*                                              -*- mode:C++ -*-
  GridTransceiver.h A variable data-rate two-way communications channel
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
  \file GridTransceiver.h A variable data-rate two-way byte communications device
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef GRIDTRANSCEIVER_H
#define GRIDTRANSCEIVER_H

#include "Mutex.h"
#include "AbstractChannel.h"

namespace MFM
{
  /**
    A model of a bidirectional communications transceiver.  Each
    direction includes an output buffer, a transmission line, an input
    buffer, and a transmission mechanism that moves output bytes over
    the transmission line to the input buffer at some given (average)
    data rate.

    In each direction, a single underlying 'ByteChannel' is used for
    all three data-carrying components, divided up by indices.  The
    initial state of a BUFFER_SIZE = 32 ByteChannel is:

    \verbatim
               1    1    2    2    3
     0....5....0....5....0....5....0.
     ^
     r
     v
     t
     w
        w: m_writeIndex == 0
        t: m_xmitIndex == 0
        v: m_rcvIndex == 0
        r: m_readIndex == 0

         CanRead()  == 0   (v+L-r)%L
         CanRcv()   == 0   (t+L-v)%L
         CanXmit()  == 0   (w+L-t)%L
         CanWrite() == 31  (r+L-w-1)%L
    \endverbatim

    Ignoring wraparound issues at first, a BUFFER_SIZE = 32
    ByteChannel could look like:

    \verbatim
               1    1    2    2    3
     0....5....0....5....0....5....0.
        abcdefghijklmnopqrstuvwxyz
        ^      ^       ^          ^
        r      v       t          w

        w: m_writeIndex == 29
        t: m_xmitIndex == 18
        v: m_rcvIndex == 10
        r: m_readIndex == 3

         CanRead()  == 7   (v+L-r)%L
         CanRcv()   == 8   (t+L-v)%L
         CanXmit()  == 11  (w+L-t)%L
         CanWrite() == 5   (r+L-w-1)%L
    \endverbatim

    If that state was following by Write("0123456789",10), the call
    would return 5, and the machine would advance to:

    \verbatim
               1    1    2    2    3
     0....5....0....5....0....5....0.
     34 abcdefghijklmnopqrstuvwxyz012
       ^^      ^       ^
       wr      v       t

        w: m_writeIndex == 2
        t: m_xmitIndex == 18
        v: m_rcvIndex == 10
        r: m_readIndex == 3

         CanRead()  == 7   (v+L-r)%L
         CanRcv()   == 8   (t+L-v)%L
         CanXmit()  == 17  (w+L-t)%L
         CanWrite() == 0   (r+L-w-1)%L
    \endverbatim

    If at that point the timer went off and the Xcvr determined that
    five more bytes should have been sent, the machine would advance to:

    \verbatim
               1    1    2    2    3
     0....5....0....5....0....5....0.
     34 abcdefghijklmnopqrstuvwxyz012
       ^^           ^       ^
       wr           v       t


        w: m_writeIndex == 2
        t: m_xmitIndex == 23
        v: m_rcvIndex == 15
        r: m_readIndex == 3

         CanRead()  == 12  (v+L-r)%L
         CanRcv()   == 8   (t+L-v)%L
         CanXmit()  == 12  (w+L-t)%L
         CanWrite() == 0   (r+L-w-1)%L
    \endverbatim

    If at that point a Read(100) call occurs, the call will return 12
    and the machine will advance to:

    \verbatim
               1    1    2    2    3
     0....5....0....5....0....5....0.
     34             mnopqrstuvwxyz012
       ^            ^       ^
       w            v       t
                    r

        w: m_writeIndex == 2
        t: m_xmitIndex == 23
        v: m_rcvIndex == 15
        r: m_readIndex == 15

         CanRead()  == 0   (v+L-r)%L
         CanRcv()   == 8   (t+L-v)%L
         CanXmit()  == 12  (w+L-t)%L
         CanWrite() == 12  (r+L-w-1)%L
    \endverbatim

    (and it's kind of bogus that CanWrite changes after a Read, since
    the whole point is that the modeled read and write locations are
    not local to each other.  Really we should have (perhaps settable)
    max read and write buffer sizes, and as long as BUFFER_SIZE is
    greater than read size + write size + max in flight, reading
    wouldn't affect writing.)

    If the timer then expires again and determines 16 bytes should
    ship, the machine advances to:

    \verbatim
               1    1    2    2    3
     0....5....0....5....0....5....0.
     34             mnopqrstuvwxyz012
       ^            ^               ^
       w            r               v
       t

        w: m_writeIndex == 2
        t: m_xmitIndex == 2
        v: m_rcvIndex == 31
        r: m_readIndex == 15

         CanRead()  == 16  (v+L-r)%L
         CanRcv()   == 3   (t+L-v)%L
         CanXmit()  == 0   (w+L-t)%L
         CanWrite() == 12  (r+L-w-1)%L
    \endverbatim

    and we observe the amount of data in-flight (CanRcv) has dropped.
    Should there be some extra delay to refill 'the air' when the
    source writes some more?  There should be a gap anyway..  But
    unless we're modeling seriously long wires with an extremely high
    data rate, it's hard to take seriously the idea of much data in
    flight.  If we ditched that possibility then we could simplify by
    combining t and v..  But I guess 'in-flight' really includes any
    source of buffering through the system, not just literally
    free-space electromagnetic propagation delays..

    So, when does the in-flight window re-expand, after it shrinks
    like this?  I guess CanRcv is bounded above by the some constant
    representing the latency and buffering through the communications
    system.  Because that data is presumed 'in-flight', there's no
    stable storage out that and so CanRcv can never be bigger than
    some value.  Therefore, whenever the machine ships bytes, t moves
    for sure -- which increases CanRcv -- and v moves as necessary to
    keep CanRcv from exceeding the in-flight data limit.  That's not
    really right, in the sense that there could be arbitrary gaps in
    flight that ought to manifest as arbitrary gaps in arriving data..
    But WTF really.

    If we set the in-flight data limit to 0, then v will track t
    exactly.

   */
  class GridTransceiver : public AbstractChannel
  {
  public:

    ////
    // BEGIN AbstractChannel interface

    /**
       \copydoc AbstractChannel::CanWrite
       \fail ILLEGAL_STATE if the GridTransceiver is not enabled
    */
    virtual u32 CanWrite(bool byA)
    {
      FailUnlessEnabled();

      Mutex::ScopeLock lock(m_access);
      return GetOutputChannel(byA).CanWrite();
    }

    /**
       \copydoc AbstractChannel::Write
       \fail ILLEGAL_STATE if the GridTransceiver is not enabled
    */
    virtual u32 Write(bool byA, const u8 * data, u32 length)
    {
      FailUnlessEnabled();

      Mutex::ScopeLock lock(m_access);
      return GetOutputChannel(byA).Write(data, length);
    }

    /**
       \copydoc AbstractChannel::CanRead
       \fail ILLEGAL_STATE if the GridTransceiver is not enabled
    */
    virtual u32 CanRead(bool byA)
    {
      FailUnlessEnabled();

      Mutex::ScopeLock lock(m_access);
      return GetInputChannel(byA).CanRead();
    }

    /**
       \copydoc AbstractChannel::Read
       \fail ILLEGAL_STATE if the GridTransceiver is not enabled
    */
    virtual u32 Read(bool byA, u8 * data, u32 length)
    {
      FailUnlessEnabled();

      Mutex::ScopeLock lock(m_access);
      return GetInputChannel(byA).Read(data, length);
    }

    // END AbstractChannel interface
    ////

    GridTransceiver() ;

    /**
       Enable or disable this GridTransceiver.  When a GridTransceiver
       is disabled, the only AbstractChannel interface method that can
       be called successfully is GetChannelState; all other interface
       will fail.

       \sa IsEnabled
     */
    void SetEnabled(bool enabled)
    {
      m_enabled = enabled;
    }

    /**
       Return true iff this GridTransceiver is enabled.

       \sa SetEnabled
     */
    bool IsEnabled() const
    {
      return m_enabled;
    }

    /**
       Simulate channel communications given the actual wall clock
       time is now.  Return true if any communications occurred.
     */
    bool AdvanceToTime(const timespec & now) ;

    /**
       Simulate channel communications as if time equal to
       nanosecondsElapsed had passed since the last call on Advance.
       Return true if any communications occurred.
     */
    bool Advance(u32 nanosecondsElapsed) ;

    void SetDataRate(u32 bytesPerSecond)
    {
      Mutex::ScopeLock lock(m_access);
      if (bytesPerSecond == 0)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      m_bytesPerSecond = bytesPerSecond;
    }

    u32 GetDataRate() const
    {
      return m_bytesPerSecond;
    }

    void SetMaxInFlight(u32 bytes)
    {
      Mutex::ScopeLock lock(m_access);
      m_maxBytesInFlight = bytes;
    }

    u32 GetMaxInFlight() const
    {
      return m_maxBytesInFlight;
    }

    u32 CanXmit(bool byA)
    {
      Mutex::ScopeLock lock(m_access);
      return GetOutputChannel(byA).CanXmit();
    }

    u32 CanRcv(bool byA)
    {
      Mutex::ScopeLock lock(m_access);
      return GetInputChannel(byA).CanRcv();
    }

  private:

    friend class GridTransceiver_Test;

    u32 IdxOf(bool byA)
    {
      return byA ? 0 : 1;
    }

    Mutex m_access;

    bool Transceive(u32 maxBytes, u32 maxInFlight)
    {
      bool didWork = false;
      if (m_enabled)
      {
        didWork |= m_channelAtoB.Transceive(maxBytes, maxInFlight);
        didWork |= m_channelBtoA.Transceive(maxBytes, maxInFlight);
      }
      return didWork;
    }

    enum {
      BUFFER_SIZE = 2048
    };

    bool m_enabled;
    void FailUnlessEnabled()
    {
      if (!m_enabled)
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    u32 m_bytesPerSecond;

    u32 m_maxBytesInFlight;

    u32 m_excessNanoseconds;  // Accumulated unused nanos from previous Advances

    timespec m_lastAdvanced;

    struct ByteChannel {

      static inline u32 BytesBetween(u32 idxHi, u32 idxLo)
      {
        return (idxHi + BUFFER_SIZE - idxLo) % BUFFER_SIZE;
      }

      static void Increment(u32 & var, u32 amount = 1)
      {
        var = (var + amount) % BUFFER_SIZE;
      }

      ByteChannel()
        : m_writeIndex(0)
        , m_xmitIndex(0)
        , m_rcvIndex(0)
        , m_readIndex(0)
      { }

      /**
       * Move xmitted bytes to rcvd bytes, and written bytes to
       * xmitted bytes; up to maxBytes each.  Return true if any bytes
       * moved.
       */
      bool Transceive(u32 maxBytes, u32 maxInFlight) ;

      u32 CanWrite()
      {
        // We waste one byte so that r==w unconditionally means empty.
        return BytesBetween(m_readIndex, m_writeIndex + 1);
      }

      u32 CanRead()
      {
        return BytesBetween(m_rcvIndex, m_readIndex);
      }

      u32 CanXmit()
      {
        return BytesBetween(m_writeIndex, m_xmitIndex);
      }

      u32 CanRcv()
      {
        return BytesBetween(m_xmitIndex, m_rcvIndex);
      }

      u32 Write(const u8 * data, u32 length) ;

      u32 Read(u8 * data, u32 length) ;

      /**
       * Next data byte to be written goes here
       */
      u32 m_writeIndex;

      /**
       * Next written byte to be transmitted is here
       */
      u32 m_xmitIndex;

      /**
       * Next transmitted byte to be received goes here
       */
      u32 m_rcvIndex;

      /**
       * Next received byte to be read is here
       */
      u32 m_readIndex;

      u8 m_data[BUFFER_SIZE];
    };
    ByteChannel & GetOutputChannel(bool byA)
    {
      return byA ? m_channelAtoB : m_channelBtoA;
    }
    ByteChannel & GetInputChannel(bool byA)
    {
      return byA ? m_channelBtoA : m_channelAtoB;
    }
    ByteChannel m_channelAtoB;
    ByteChannel m_channelBtoA;

  };
}

#endif /* GRIDTRANSCEIVER_H */
