/*                                              -*- mode:C++ -*-
  ThreadQueue.h Thread-Safe byte buffer
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
  \file ThreadQueue.h Thread-Safe byte buffer
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef QUE_H
#define QUE_H

#include "itype.h"
#include "Mutex.h"
#include "Logger.h"

#define THREADQUEUE_MAX_BYTES (2048)   // How big does this actually need to be?

namespace MFM
{
  /**
   * A thread-safe queue implementation, backed by a byte buffer of
   * fixed length.
   */
  class ThreadQueue
  {
  private:

    /**
     * The mutex required to lock this ThreadQueue upon
     * reading and writing.
     */
    Mutex m_mutex;

    /**
     * A Mutex::Predicate that waits for additional input to be available
     */
    struct MoreInputIsAvailable : public Mutex::Predicate
    {
      ThreadQueue & m_threadQueue;
      MoreInputIsAvailable(ThreadQueue & tq) : Predicate(tq.m_mutex), m_threadQueue(tq) { }

      virtual bool EvaluatePrecondition()
      {
        bool ret = m_threadQueue.m_heldBytes == 0;
        if (!ret)
        {
          LOG.Error("MoreInputIsAvailable precondition failed: m_heldBytes = %d",
		    m_threadQueue.m_heldBytes);
        }
        return ret;
      }

      virtual bool EvaluatePredicate()
      {
        return m_threadQueue.m_heldBytes > 0;
      }
    } m_moreInputIsAvailable;

    /**
     * The actual data held within this ThreadQueue.
     */
    u8 m_queueData[THREADQUEUE_MAX_BYTES];

    /**
     * The location within m_queueData which the next byte received
     * through Write() will be written to.
     *
     * @sa Write
     */
    u32 m_writeHead;

    /**
     * The location within m_queueData which the next byte asked for
     * by Read() will be read from.
     *
     * @sa Read
     */
    u32 m_readHead;

    /**
     * The number of bytes currently available to be read from
     * m_queueData.
     */
    u32 m_heldBytes;

    /**
     * Reads a specified number of bytes from this ThreadQueue into a
     * specified buffer. These bytes are taken directly from the front
     * of the underlying queue. This does not block, and instead reads
     * as many bytes as possible from the underlying queue, up to the
     * specified limit, before returning.
     *
     * ****************NOTICE*******************
     * This function ignores the mutex and is meant to be for internal
     * use only!
     * *****************NOTICE*******************
     *
     * @param bytes The buffer to read bytes into. This buffer will be
     *              overwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     *
     */
    u32 UnsafeRead(u8* bytes, u32 length);

  public:

    /**
     * Constructs a new ThreadQueue, setting up all mutex related
     * structures.
     */
    ThreadQueue();

    /**
     * Deconstructs a ThreadQueue, destroying all mutexes.
     */
    ~ThreadQueue();

    /**
     * Writes a specified number of bytes to this ThreadQueue from a
     * specified buffer. If writing is not available, this thread
     * waits until it is.
     *
     * @param bytes The buffer where bytes will be taken from during
     *              writing.
     *
     * @param length The number of bytes which will be taken from
     *               bytes. The caller must ensure that this number is
     *               less than or equal to the length of bytes.
     */
    void Write(u8* bytes, u32 length);

    /**
     * Reads a specified number of bytes from this ThreadQueue into a
     * specified buffer. These bytes are taken directly from the front
     * of the underlying queue. This blocks the calling thread until
     * all bytes have been read.
     *
     * @param bytes The buffer to read bytes into. This buffer will be
     *              overrwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     */
    void ReadBlocking(u8* bytes, u32 length);

    /**
     * Reads a specified number of bytes from this ThreadQueue into a
     * specified buffer. These bytes are taken directly from the front
     * of the underlying queue. This does not block, and instead reads
     * as many bytes as possible from the underlying queue, up to the
     * specified limit, before returning.
     *
     * @param bytes The buffer to read bytes into. This buffer will be
     *              overrwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     *
     * @returns The number of bytes read successfully from this call.
     */
    u32 Read(u8* bytes, u32 length);

    /**
     * Writes a series of held bytes to a specified buffer. This FAILs
     * with ARRAY_INDEX_OUT_OF_BOUNDS if reading will go over the
     * bounds of this ThreadQueue .
     *
     * @param toBuffer The buffer to read bytes from this ThreadQueue
     *                 into.
     *
     * @param index The byte index to begin reading from.
     *
     * @param length The number of bytes to read from this ThreadQueue .
     */
    void PeekRead(u8* toBuffer, u32 index, u32 length);

    /**
     * Gets the number of bytes currently ready to be read from the
     * underlying queue.
     *
     * @returns The number of bytes ready to be read from the
     *          underlying queue.
     */
    u32 BytesAvailable() ;

    /**
     * Erases all held data within this TheadQueue. This should only
     * be called in erroneous cases where the ThreadQueue should no
     * longer need to hold any data inside. This should NEVER be used
     * in tandem with a call to ReadBlocking; this will create a
     * deadlock.
     */
    void Flush();

  };
}

#endif /* QUE_H */
