#ifndef QUE_H /* -*- C++ -*- */
#define QUE_H

#include "itype.h"
#include <pthread.h>

#define THREADQUEUE_MAX_BYTES 128

namespace ThreadSpike
{
  class ThreadQueue
  {
  private:

    pthread_mutex_t m_lock;

    pthread_cond_t m_cond;

    u8 m_queueData[THREADQUEUE_MAX_BYTES];

    u32 m_readHead, m_writeHead, m_heldBytes;

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
     *              overrwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     *
     */
    u32 UnsafeRead(u8* bytes, u32 length);

  public:

    ThreadQueue();

    ~ThreadQueue();

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
     * Gets the number of bytes currently ready to be read from the
     * underlying queue.
     *
     * @returns The number of bytes ready to be read from the
     *          underlying queue.
     */
    u32 BytesAvailable();

  };
}

#include "que.tcc"

#endif /* QUE_H */
