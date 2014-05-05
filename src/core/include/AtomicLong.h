 /* -*- C++ -*- */

#ifndef ATOMICLONG_H
#define ATOMICLONG_H

#include "itype.h"
#include <pthread.h>

namespace MFM
{
  /**
   * A thread-safe implementation of an unsigned 64-bit integer.
   */
  class AtomicLong
  {
  private:

    /**
     * The pthread_mutex_t used to allow only a single thread to
     * observe or modify the state of this AtomicLong.
     */
    pthread_mutex_t m_lock;

    /**
     * The pthread_cont_t used to alert any blocked threads waiting on
     * this AtomicLong's state that they may observe or modify it.
     */
    pthread_cond_t m_cond;

    /**
     * The 64-bit unsigned value that this AtomicLong represents.
     */
    u64 m_long;

  public:

    /**
     * Constructs a new AtomicLong, and therefore initializes all
     * threading constructs.
     *
     * @remarks This may fail depending on the state of the pthread
     * library.
     */
    AtomicLong();

    /**
     * Destroys this AtomicLong, including all held threading
     * constructs.
     */
    ~AtomicLong();

    /**
     * Adds a value to this AtomicLong's state.
     *
     * @param value The value to add to this AtomicLong.
     *
     * @remarks A calling thread will block momentarily if another
     *          thread is attempting to modify this AtomicLong.
     */
    void Add(u64 value);

    /**
     * Adds one (1) to this AtomicLong's state.
     *
     * @remarks A calling thread will block momentarily if another
     *          thread is attempting to modify this AtomicLong.
     */
    void Increment();

    /**
     * Gets the value of this AtomicLong and resets its internal state
     * to zero (0).
     *
     * @returns The state that this AtomicLong held before calling
     * this method.
     *
     * @remarks A calling thread will block momentarily if another
     *          thread is attempting to modify this AtomicLong.
     */
    u64 GetAndReset();

  };
}

#endif /* ATOMICLONG_H */
