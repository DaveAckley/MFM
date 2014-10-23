#include "ThreadPauser.h"
#include <assert.h>      /* For assert */

namespace MFM
{
  ThreadPauser::ThreadPauser() :
    m_stateIsRunRequested(*this),
    m_stateIsRunning(*this)
  {
    m_threadState = THREADSTATE_PAUSED;
    m_threadStatePrevious = (ThreadState) -1; // Invalid previous threadstate
    m_threadStatePreviousOuter = false;
    m_threadStatePreviousThreadId = pthread_self();
  }

  ThreadPauser::~ThreadPauser()
  {
  }

  ThreadState ThreadPauser::AdvanceStateOuter(ThreadState fromState)
  {
    Mutex::ScopeLock lock(m_mutex);  // Hold the lock during this block

    if(m_ignoreThreadingProblems)
    {
      if(m_threadState != fromState)
      {
        LOG.Error("%s:%d: THREADING PROBLEM ENCOUNTERED! ThreadPauser is configured to"
                  " ignore this problem and will continue execution.", __FILE__, __LINE__);
      }
    }
    else
    {
      assert(m_threadState == fromState);
    }

    switch (m_threadState)
    {

    case THREADSTATE_RUN_REQUESTED:
      if(m_ignoreThreadingProblems)
      {
        LOG.Error("Pretending THREADSTATE_RUN_REQUESTED is THREADSTATE_RUN_READY");
      }
      else
      {
        assert(false);
      }
      /* FALL THROUGH */

    case THREADSTATE_RUN_READY:

      m_threadStatePrevious = m_threadState;
      m_threadStatePreviousOuter = true;
      m_threadStatePreviousThreadId = pthread_self();

      m_threadState = THREADSTATE_RUNNING;
      m_stateIsRunning.SignalCondition();
      break;

    case THREADSTATE_RUNNING:

      m_threadStatePrevious = m_threadState;
      m_threadStatePreviousOuter = true;
      m_threadStatePreviousThreadId = pthread_self();

      m_threadState = THREADSTATE_PAUSE_REQUESTED;
      break;

    case THREADSTATE_PAUSE_REQUESTED:
      if(m_ignoreThreadingProblems)
      {
        LOG.Error("Pretending THREADSTATE_PAUSE_REQUESTED is THREADSTATE_PAUSE_READY");
      }
      else
      {
        assert(false);
      }
      /* FALL THROUGH */

    case THREADSTATE_PAUSE_READY:

      m_threadStatePrevious = m_threadState;
      m_threadStatePreviousOuter = true;
      m_threadStatePreviousThreadId = pthread_self();

      m_threadState = THREADSTATE_PAUSED;
      break;

    case THREADSTATE_PAUSED:

      m_threadStatePrevious = m_threadState;
      m_threadStatePreviousOuter = true;
      m_threadStatePreviousThreadId = pthread_self();

      m_threadState = THREADSTATE_RUN_REQUESTED;
      m_stateIsRunRequested.SignalCondition();
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
      {
        m_threadStatePrevious = m_threadState;
        m_threadStatePreviousOuter = false;
        m_threadStatePreviousThreadId = pthread_self();

        m_threadState = THREADSTATE_PAUSE_READY;
      }
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
      {
        m_threadStatePrevious = m_threadState;
        m_threadStatePreviousOuter = false;
        m_threadStatePreviousThreadId = pthread_self();

        m_threadState = THREADSTATE_RUN_READY;
      }
      break;

    default:
      assert(false);
    }

    return m_threadState; // Return current state and release lock
  }

  const char * ThreadPauser::GetThreadStateName(ThreadState ts)
  {
    switch (ts)
    {
    case THREADSTATE_PAUSED: return "THREADSTATE_PAUSED";
    case THREADSTATE_RUN_REQUESTED: return "THREADSTATE_RUN_REQUESTED";
    case THREADSTATE_RUN_READY: return "THREADSTATE_RUN_READY";
    case THREADSTATE_RUNNING: return "THREADSTATE_RUNNING";
    case THREADSTATE_PAUSE_REQUESTED: return "THREADSTATE_PAUSE_REQUESTED";
    case THREADSTATE_PAUSE_READY: return "THREADSTATE_PAUSE_READY";
    default: return "invalid ThreadState";
    }
  }

  void ThreadPauser::ReportThreadPauserStatus(Logger::Level level)
  {
    LOG.Log(level,"   =ThreadPauser %p=", (void*) this);
    LOG.Log(level,"   =Current pthread id: %p", (void*) pthread_self());
    LOG.Log(level,"   =Current ThreadState: %d (%s)",
            (int) m_threadState, GetThreadStateName(m_threadState));
    LOG.Log(level,"   =Previous ThreadState: %d (%s)",
            (int) m_threadStatePrevious, GetThreadStateName(m_threadStatePrevious));
    LOG.Log(level,"   =Previous ThreadState changed by: %s",
            m_threadStatePreviousOuter ? "Outer" : "Inner");
    LOG.Log(level,"   =Previous ThreadState changed by thread id: %p",
            (void*) m_threadStatePreviousThreadId);

    m_mutex.ReportMutexStatus(level);
  }
}
