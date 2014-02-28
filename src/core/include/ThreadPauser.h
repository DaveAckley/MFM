#ifndef THREAD_PAUSER_H    /* -*- C++ -*- */
#define THREAD_PAUSER_H

#include <pthread.h>

namespace MFM
{
  /**
   * A threading construct which pauses a looping thread by blocking it.
   */
  class ThreadPauser
  {
  private:

    /**
     * The mutex required to lock the looping thread.
     */
    pthread_mutex_t m_lock;

    /**
     * The thread condition required to lock the looping thread.
     */
    pthread_cond_t m_pauseCond;

    /**
     * The thread condition used to announce to the controlling thread
     * that the looping thread is paused.
     */
    pthread_cond_t m_joinCond;

    /**
     * A boolean, describing whether or not the looping thread is
     * paused.
     */
    bool m_paused;

  public:

    /**
     * Constructs a new ThreadPauser.
     */
    ThreadPauser();

    /**
     * Destroys this ThreadPauser.
     */
    ~ThreadPauser();

    /**
     * Finds out if this ThreadPauser is actively trying to pause the
     * looping thread.
     */
    bool IsPaused();

    /**
     * To be called by the looping thread. This pauses the thread if it
     * is supposed to be paused.
     */
    void WaitIfPaused();

    /**
     * To be called by the controlling thread. This returns as soon as
     * the looping thread is waiting.
     */
    void PauseBlocking();

    /**
     * To be called by the controlling thread. This returns immediately
     * and pauses the looping thread.
     */
    void Pause();

    /**
     * To be called by the controlling thread. This wakes up the looping
     * thread if it is paused.
     */
    void Unpause();
  };
}

#endif /* THREAD_PAUSER_H */
