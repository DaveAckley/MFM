/*                                              -*- mode:C++ -*-
  Connection.h Multithread communicator
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
  \file Connection.h Multithread communicator
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CONNECTION_H
#define CONNECTION_H

#include "ThreadQueue.h"
#include "Mutex.h"
#include "itype.h"
#include "Logger.h"
#include <assert.h>

namespace MFM
{
  /**
   * A construct meant to be used by two threads as a communication
   * channel, supporting reading and writing on both ends. Only one
   * thread is able to access the IO of this construct at a time,
   * allowing for thread safe operation.
   */
  class Connection
  {
  private:

    /**
     * A Mutex controlling access to this Connection .
     */
    Mutex m_lock;

    /**
     * The two ThreadQueue constructs used for two way IO .
     */
    ThreadQueue m_outbuffer, m_inbuffer;

    /**
     * A flag which indicates whether or not this Connection is
     * currently established and functional.
     */
    bool m_connected;

  public:

    /**
     * Creates a new Connection which is not connected. Also
     * initializes the internal mutex.
     */
    Connection()
    {
      m_connected = false;
    }

    /**
     * Deconstructs this Connection, freeing the internal mutex.
     */
    ~Connection()
    {
    }

    /**
     * Sets the connection status of this Connection, allowing or
     * disallowing communication.
     *
     * @param value If \c true , will allow this Connection to be
     *              written to and read from. Else, will disallow
     *              these features.
     */
    void SetConnected(bool value)
    {
      m_connected = value;
    }

    /**
     * Checks whether or not this Connection is able to be used as a
     * communication channel.
     *
     * @returns \c true if this Connection is able to be written to
     *          and read from on both ends, else \c false .
     */
    bool IsConnected()
    {
      return m_connected;
    }

    /** Test if the connection is currently locked by a thread other
     * than the current thread.
     *
     * FAILs with LOCK_FAILURE if the connection is locked by the
     * current thread.
     *
     * @returns true if the lock is currently held by a thread other
     * than the current thread, and false if the lock is currently
     * unlocked
     *
     */
    bool IsLockedByAnother()
    {
      return m_lock.IsLockedByAnother();
    }

    /** Locks the underlying mutex, allowing for thread-safe execution
     * along this connection.
     *
     * @returns true if the lock was successfully taken, else false.
     */
    bool Lock()
    {
      /*
      if (m_lock.IHoldThisLock()) {
        LOG.Debug("Connection %p already owns lock",(void*) this);
        return false;
      }
      */
      return m_lock.TryLock();
    }

    /**
     * Unlocks the underlying mutex, allowing for another thread to lock
     * this Connection. DO NOT call this method from a thread which does
     * not already hold the lock!
     */
    void Unlock()
    {
      m_lock.Unlock();
    }

    /**
     * Waits until this Connection is not busy, then reads a specified
     * amount of bytes into a specified buffer. This method blocks the
     * calling thread until it is able to take control of this
     * Connection.
     *
     * @param buffer The buffer to write bytes read from this
     *               Connection to.
     *
     * @param length The number of bytes which will be written to \c
     *               buffer before this method returns.
     */
    void ReadBlocking(bool child, u8* buffer, u32 length)
    {
      ThreadQueue& queue = child ? m_outbuffer : m_inbuffer;

      queue.ReadBlocking(buffer, length);
    }

    /**
     * Reads from the corresponding underlying queue without blocking
     * the calling thread.
     *
     * @param child This should be true if the calling thread is not
     *              the owner of this Connection.
     *
     * @param buffer The location to read bytes into.
     *
     * @param length The number of bytes to read from the underlying buffer.
     *               Use 0 if wanting to read as many as possible.
     *
     * @returns The number of bytes successfully read from this Connection.
     *
     * @sa Write
    */
    u32 Read(bool child, u8* buffer, u32 length)
    {
      ThreadQueue& queue = child ? m_outbuffer : m_inbuffer;

      return queue.Read(buffer, length);
    }

    /**
     * Reads a series of bytes from an underlying queue without
     * blocking the calling thread.  This FAILs with
     * ARRAY_INDEX_OUT_OF_BOUNDS if reading will go over the bounds of
     * this ThreadQueue. This does not consume any bytes from the ThreadQueue .
     */
    void PeekRead(bool output, u8* buffer, u32 index, u32 length)
    {
      ThreadQueue& queue = output ? m_outbuffer : m_inbuffer;

      queue.PeekRead(buffer, index, length);
    }

    /**
     * Writes a series of bytes to a specified internal buffer of this
     * Connection.
     *
     * @param child Selects which internal buffer to write to. If the
     *              calling object created this Connection, use \c
     *              true . If not, use \c false .
     *
     * @param buffer The buffer containing the bytes to write to this
     *               Connection.
     *
     * @param length The number of bytes which will be written to this
     *               Connection from \c buffer before this method
     *               returns.
     */
    void Write(bool child, u8* buffer, u32 length)
    {
      ThreadQueue& queue = !child ? m_outbuffer : m_inbuffer;

      queue.Write(buffer, length);
    }

    /**
     * Number of bytes currently in the input buffer of this
     * connection.
     */
    u32 InputByteCount()
    {
      return m_inbuffer.BytesAvailable();
    }

    /**
     * Number of bytes currently in the output buffer of this
     * connection.
     */
    u32 OutputByteCount()
    {
      return m_outbuffer.BytesAvailable();
    }

    void LogAllPackets()
    {

    }

    void ReportConnectionStatus(Logger::Level level, bool owned) ;

  };
}

#endif /* CONNECTION_H */
