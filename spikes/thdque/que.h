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
    
    u8 m_queueData[THREADQUEUE_MAX_BYTES];

    u32 m_readHead, m_writeHead, m_heldBytes;
    
  public:
    
    ThreadQueue();

    ~ThreadQueue();

    void Write(u8* bytes, u32 length);
    
    /* Returns the number of bytes read. */
    u32 Read(u8* bytes, u32 length);

    u32 BytesAvailable();
    
  };
}

#include "que.tcc"

#endif /* QUE_H */
