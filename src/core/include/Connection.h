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
#include "itype.h"

namespace MFM
{

  class Connection
  {
  private:
    pthread_mutex_t m_lock;

    ThreadQueue m_outbuffer, m_inbuffer;

    bool m_connected;

  public:

    Connection()
    {
      m_connected = false;
      pthread_mutex_init(&m_lock, NULL);
    }

    ~Connection()
    {
      pthread_mutex_destroy(&m_lock);
    }

    void SetConnected(bool value)
    {
      m_connected = value;
    }

    bool IsConnected()
    {
      return m_connected;
    }

    /** Locks the underlying mutex, allowing for thread-safe execution
     * along this connection.
     *
     * @returns true if the lock was successfully taken, else false.
     */
    bool Lock()
    {
      return !pthread_mutex_trylock(&m_lock);
    }

    /**
     * Unlocks the underlying mutex, allowing for another thread to lock
     * this Connection. DO NOT call this method from a thread which does
     * not already hold the lock!
     */
    void Unlock()
    {
      pthread_mutex_unlock(&m_lock);
    }

    void ReadBlocking(bool child, u8* buffer, u32 length)
    {
      ThreadQueue& queue = child ? m_outbuffer : m_inbuffer;

      queue.ReadBlocking(buffer, length);
    }

    /**
     * Reads from the corresponding underlying queue.
     *
     * @param child This should be true if the calling thread is not
     *              the owner of this Connection.
     *
     * @param buffer The location to read bytes into.
     *
     * @param length The number of bytes to read from the underlying buffer.
     *               Use 0 if wanting to read as many as possible.
     @sa Write
    */
    u32 Read(bool child, u8* buffer, u32 length)
    {
      ThreadQueue& queue = child ? m_outbuffer : m_inbuffer;

      return queue.Read(buffer, length);
    }

    void Write(bool child, u8* buffer, u32 length)
    {
      ThreadQueue& queue = !child ? m_outbuffer : m_inbuffer;

      queue.Write(buffer, length);
    }

  };
}

#endif /* CONNECTION_H */
