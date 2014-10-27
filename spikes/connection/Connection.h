#ifndef CONNECTION_H
#define CONNECTION_H

#include "../thdque/que.h"
#include "../thdque/itype.h"

class Connection
{
private:
  pthread_mutex_t m_lock;
  
  ThreadSpike::ThreadQueue m_outbuffer, m_inbuffer;

public:
  
  Connection()
  {
    pthread_mutex_init(&m_lock, NULL);
  }

  ~Connection()
  {
    pthread_mutex_destroy(&m_lock);
  }

  bool Lock()
  {
    return !pthread_mutex_trylock(&m_lock);
  }

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
    ThreadSpike::ThreadQueue& queue = child ? m_outbuffer : m_inbuffer;

    return queue.Read(buffer, length);
  }

  void Write(bool child, u8* buffer, u32 length)
  {
    ThreadSpike::ThreadQueue& queue = !child ? m_outbuffer : m_inbuffer;

    queue.Write(buffer, length);
  }
  
};

#endif /* CONNECTION_H */
