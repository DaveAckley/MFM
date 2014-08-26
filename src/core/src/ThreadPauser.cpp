#include "ThreadPauser.h"
#include <assert.h>      /* For assert */

namespace MFM
{
  ThreadPauser::ThreadPauser() :
    m_stateIsRunRequested(*this),
    m_stateIsRunning(*this)
  {
    m_threadState = THREADSTATE_PAUSED;
  }

  ThreadPauser::~ThreadPauser()
  {
  }

  ThreadState ThreadPauser::AdvanceStateOuter(ThreadState fromState)
  {
    Mutex::ScopeLock lock(m_mutex);  // Hold the lock during this block

    assert(m_threadState == fromState);

    switch (m_threadState)
    {

    case THREADSTATE_RUN_READY:
      m_threadState = THREADSTATE_RUNNING;
      m_stateIsRunning.SignalCondition();
      break;

    case THREADSTATE_RUNNING:
      m_threadState = THREADSTATE_PAUSE_REQUESTED;
      break;

    case THREADSTATE_PAUSE_REQUESTED:
      assert(false);
      break;

    case THREADSTATE_PAUSE_READY:
      m_threadState = THREADSTATE_PAUSED;
      break;

    case THREADSTATE_PAUSED:
      m_threadState = THREADSTATE_RUN_REQUESTED;
      m_stateIsRunRequested.SignalCondition();
      break;

    case THREADSTATE_RUN_REQUESTED:
      assert(false);
      break;

    default:
      assert(false);
    }

    return m_threadState; // Return (new) current state and release lock
  }

  ThreadState ThreadPauser::GetStateNonblocking()
  {
    Mutex::ScopeLock lock(m_mutex);  // Hold the lock during this block
    return m_threadState;
  }

  ThreadState ThreadPauser::GetAdvanceStateInner(bool innerReadyToAdvance)
  {
    Mutex::ScopeLock lock(m_mutex);  // Hold the lock during this block

    switch (m_threadState)
    {

    case THREADSTATE_RUN_READY:
      assert(!innerReadyToAdvance);
      m_stateIsRunning.WaitForCondition();  // Release lock and block until running
      break;

    case THREADSTATE_RUNNING:
      assert(!innerReadyToAdvance);
      // Just get back to doing events until Outer calls RequestPause()
      break;

    case THREADSTATE_PAUSE_REQUESTED:
      // Inner finishes current event, including getting ACKs from any
      // neighbors it has locked, then sets innerReadyToAdvance
      if (innerReadyToAdvance)
        m_threadState = THREADSTATE_PAUSE_READY;
      break;

    case THREADSTATE_PAUSE_READY:
      assert(!innerReadyToAdvance);
      // Now process other people's packets until Outer calls Pause()
      break;

    case THREADSTATE_PAUSED:
      assert(!innerReadyToAdvance);
      m_stateIsRunRequested.WaitForCondition();  // Release lock and block until run request
      break;

    case THREADSTATE_RUN_REQUESTED:
      // Inner acks the run request by setting innerReadyToAdvance here
      if (innerReadyToAdvance)
        m_threadState = THREADSTATE_RUN_READY;
      break;

    default:
      assert(false);
    }

    return m_threadState; // Return current state and release lock
  }

  /*
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

  void ThreadPauser::Pause()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_PAUSE_READY);
      m_threadState = THREADSTATE_PAUSED;
    }
    pthread_mutex_unlock(&m_lock);
  }

  void ThreadPauser::RequestRun()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_PAUSED);
      m_threadState = THREADSTATE_RUN_REQUESTED;
    }
    pthread_mutex_unlock(&m_lock);
  }


  void ThreadPauser::Run()
  {
    pthread_mutex_lock(&m_lock);
    {
      assert(m_threadState == THREADSTATE_RUN_READY);
      m_threadState = THREADSTATE_RUNNING;
      pthread_cond_signal(&m_pauseCond);
    }
    pthread_mutex_unlock(&m_lock);
  }
  */
}
