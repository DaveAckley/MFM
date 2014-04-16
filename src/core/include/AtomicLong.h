 /* -*- C++ -*- */

#ifndef ATOMICLONG_H
#define ATOMICLONG_H

#include "itype.h"
#include <pthread.h>

namespace MFM
{
  class AtomicLong
  {
  private:

    pthread_mutex_t m_lock;

    pthread_cond_t m_cond;

    u64 m_long;

  public:

    AtomicLong();

    ~AtomicLong();
    
    void Add(u64 value);
    
    void Increment();

    u64 GetAndReset();

  };
}

#endif /* ATOMICLONG_H */
