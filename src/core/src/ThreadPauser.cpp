#include "ThreadPauser.h"
#include <assert.h>      /* For assert */

namespace MFM
{
  ThreadPauser::ThreadPauser()
  {
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_pauseCond, NULL);
    pthread_cond_init(&m_joinCond, NULL);

    m_threadState = THREADSTATE_PAUSED;  // Able to be configured; ready for an initial unpause
  }

  ThreadPauser::~ThreadPauser()
  {
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_pauseCond);
    pthread_cond_destroy(&m_joinCond);
  }

  bool ThreadPauser::IsPaused()
  {
    bool retval;
    pthread_mutex_lock(&m_lock);
    {
      retval = (m_threadState == THREADSTATE_PAUSED);
    }
    pthread_mutex_unlock(&m_lock);

    return retval;
  }

  bool ThreadPauser::IsPauseRequested()
  {
    bool retval;
    pthread_mutex_lock(&m_lock);
    {
      retval = (m_threadState == THREADSTATE_PAUSE_REQUESTED);
    }
    pthread_mutex_unlock(&m_lock);

    return retval;
  }

  bool ThreadPauser::IsPauseReady()
  {
    bool retval;
    pthread_mutex_lock(&m_lock);
    {
      retval = (m_threadState == THREADSTATE_PAUSE_READY);
    }
    pthread_mutex_unlock(&m_lock);

    return retval;
  }

  void ThreadPauser::PauseReady()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_PAUSE_REQUESTED);
      m_threadState = THREADSTATE_PAUSE_READY;
    }
    pthread_mutex_unlock(&m_lock);
  }

  void ThreadPauser::RequestPause()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_RUNNING);
      m_threadState = THREADSTATE_PAUSE_REQUESTED;
    }
    pthread_mutex_unlock(&m_lock);
  }

  void ThreadPauser::WaitIfPaused()
  {
    if(IsPaused())
    { /* Don't bother grabbing the lock unless we're sure about being paused. */
      pthread_mutex_lock(&m_lock);
      {
        while(m_threadState == THREADSTATE_PAUSED)
        {
          pthread_cond_signal(&m_joinCond);
          pthread_cond_wait(&m_pauseCond, &m_lock);
        }
      }
      pthread_mutex_unlock(&m_lock);
    }
  }

  void ThreadPauser::Pause()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_PAUSE_READY);
      m_threadState = THREADSTATE_PAUSED;
    }
    pthread_mutex_unlock(&m_lock);
  }

  void ThreadPauser::Unpause()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_PAUSED);
      m_threadState = THREADSTATE_RUNNING;
      pthread_cond_signal(&m_pauseCond);
    }
    pthread_mutex_unlock(&m_lock);
  }
}
