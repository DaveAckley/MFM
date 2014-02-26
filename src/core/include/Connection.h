#ifndef CONNECTION_H  /* -*- C++ -*- */
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
