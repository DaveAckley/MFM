#include <stdio.h>    /* -*- C++ -*- */
#include <stdlib.h>
#include "AtomicLong.h"

#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))

namespace MFM
{
  AtomicLong::AtomicLong()
  {
    m_long = 0;
    if(pthread_mutex_init(&m_lock, NULL))
    {
      fprintf(stderr, "ERROR: Mutex did not initialize.\n");
      exit(1);
    }
    if(pthread_cond_init(&m_cond, NULL))
    {
      fprintf(stderr, "ERROR: Cond did not initialize.\n");
      exit(2);
    }
  }

  AtomicLong::~AtomicLong()
  {
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_cond);
  }

  void AtomicLong::Add(u64 amount)
  {
    pthread_mutex_lock(&m_lock);
    {
      m_long += amount;
    }
    pthread_cond_signal(&m_cond); /* Reading is available! */
    pthread_mutex_unlock(&m_lock);
  }

  void AtomicLong::Increment()
  {
    Add(1);
  }

  u64 AtomicLong::GetAndReset()
  {
    u64 val;
    pthread_mutex_lock(&m_lock);
    {
      val = m_long;
      m_long = 0;
    }
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_lock);
    return val;
  }
}
