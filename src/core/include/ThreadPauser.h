/*                                              -*- mode:C++ -*-
  ThreadPauser.h Structure allowing a thread to pause another
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file ThreadPauser.h Structure allowing a thread to pause another
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef THREAD_PAUSER_H
#define THREAD_PAUSER_H

#include "Mutex.h"
#include "Logger.h"

namespace MFM
{
  /**
   * The states of the ThreadPauser state machine
   */
  enum ThreadState
  {
    /**
     * Prev state: THREADSTATE_PAUSE_READY, also none
     * This state: THREADSTATE_PAUSED
     * Next state: THREADSTATE_RUN_REQUESTED
     *
     * Precondition:  All buffers are empty and no intertile locks are
     *                held in the whole grid
     * Entered by:    Outer advances from THREADSTATE_PAUSE_READY; also
     *                initial state from ctor
     * Behavior:      Inner waits on condition variable m_pause, for
     *                the predicate state==THREADSTATE_RUN_REQUESTED.
     *                Outer is free to access and modify all atomic
     *                data in all Tiles.
     * Exited by:     Outer calls RequestRun() and signals condition
     *                variable m_pause
     * Postcondition: Inner is awakened.  Outer is not allowed to
     *                access Tile's atomic data.
     */
    THREADSTATE_PAUSED,

    /**
     * Prev state: THREADSTATE_PAUSED
     * This state: THREADSTATE_RUN_REQUESTED
     * Next state: THREADSTATE_RUN_READY
     *
     * Precondition:  Tile buffers are empty and no intertile locks are held
     * Entered by:    Outer calls RequestRun() and signals condvar m_pause
     * Behavior:      Inner awakens and detects state is RUN_REQUESTED
     * Exited by:     Inner calls RunReady()
     * Postcondition: Tile buffers are empty and no intertile locks are held
     */
    THREADSTATE_RUN_REQUESTED,

    /**
     * Prev state: THREADSTATE_RUN_REQUESTED
     * This state: THREADSTATE_RUN_READY
     * Next state: THREADSTATE_RUNNING
     *
     * Precondition:  Tile buffers are empty and no intertile locks are held
     * Entered by:    Inner calls RunReady()
     * Behavior:      Inner waits on condition variable m_run for predicate state == RUNNING
     * Exited by:     Outer calls Run() and signals condition var m_run
     * Postcondition: State is THREADSTATE_RUNNING and Inner is awake.
     */
    THREADSTATE_RUN_READY,

    /**
     * Prev state: THREADSTATE_RUN_READY
     * This state: THREADSTATE_RUNNING
     * Next state: THREADSTATE_PAUSE_REQUESTED
     *
     * Precondition:  All Tiles are THREADSTATE_RUN_READY and all Inner
     *                threads are waiting on condition var m_run
     * Entered by:    Outer calls Run() and signals condition var m_run
     * Behavior:      Inner loops asynchronously performing events
     * Exited by:     Outer calls RequestPause()
     * Postcondition: ? State is THREADSTATE_PAUSE_REQUESTED
     */
    THREADSTATE_RUNNING,

    /**
     * Prev state: THREADSTATE_RUNNING
     * This state: THREADSTATE_PAUSE_REQUESTED
     * Next state: THREADSTATE_PAUSE_READY
     *
     * Precondition:  ? State is THREADSTATE_PAUSE_REQUESTED
     * Entered by:    Outer calls RequestPause()
     * Behavior:      Inner finishes current event, including getting ACKs
     *                from on any connections it has locked
     * Exited by:     Inner advances state to THREADSTATE_PAUSE_READY when
     *                it is holding no connection locks
     * Postcondition: Inner holds no connection locks
     */
    THREADSTATE_PAUSE_REQUESTED,

    /**
     * Prev state: THREADSTATE_PAUSE_REQUESTED
     * This state: THREADSTATE_PAUSE_READY
     * Next state: THREADSTATE_PAUSED
     *
     * Precondition:  All connections known to Inner are unlocked
     * Entered by:    Inner advanced state to THREADSTATE_PAUSE_READY
     * Behavior:      Processes packets from any connections locked by
     *                its neighbors.
     * Exited by:     Outer advances state after it observes that all
     *                Tiles report THREADSTATE_PAUSE_READY
     * Postcondition: All output buffers (in the whole grid wide) --
     *                and therefore all input buffers -- are empty,
     *                and no one holds any intertile locks).
     */
    THREADSTATE_PAUSE_READY

  };

  /**
   * A threading construct which pauses a looping thread by blocking
   * it. It is controlled by what is known as an 'outer' thread
   * (i.e. a thread which tells another to pause), and it controls an
   * 'inner' thread (i.e. the thread to be paused).
   */
  class ThreadPauser
  {
   private:

    /**
     * The mutex gating all access to the ThreadPauser state
     */
    Mutex m_mutex;

    /**
     * The ThreadPauser current state
     */
    ThreadState m_threadState;

    /**
     * The ThreadPauser previous state (for consistency checking)
     */
    ThreadState m_threadStatePrevious;

    /**
     * True if the last state change was officially by outer
     */
    bool m_threadStatePreviousOuter;

    /**
     * Thread id prevailing at the last official state change
     */
    pthread_t m_threadStatePreviousThreadId;

    /**
     * If /c true , will ignore problems encountered with inconsistent
     * threading.
     */
    bool m_ignoreThreadingProblems;

    /**
     * A Mutex::Predicate that waits for THREADSTATE_RUN_REQUESTED
     */
    struct StateIsRunRequested : public Mutex::Predicate
    {
      ThreadPauser & m_threadPauser;
      StateIsRunRequested(ThreadPauser & tp) :
        Predicate(tp.m_mutex),
        m_threadPauser(tp)
      { }

      virtual bool EvaluatePrecondition()
      {
        bool ret = m_threadPauser.m_threadState == THREADSTATE_PAUSED;
        if (!ret)
        {
          LOG.Error("PAUSED precondition failed");
          m_threadPauser.ReportThreadPauserStatus(Logger::ERROR);
        }
        return ret;
      }

      virtual bool EvaluatePredicate()
      {
        return m_threadPauser.m_threadState == THREADSTATE_RUN_REQUESTED;
      }
    } m_stateIsRunRequested;

    /**
     * A Mutex::Predicate that waits for THREADSTATE_RUNNING
     */
    struct StateIsRunning : public Mutex::Predicate
    {
      ThreadPauser & m_threadPauser;
      StateIsRunning(ThreadPauser & tp) :
        Predicate(tp.m_mutex),
        m_threadPauser(tp)
      { }

      virtual bool EvaluatePrecondition()
      {
        bool ret = m_threadPauser.m_threadState == THREADSTATE_RUN_READY;
        if (!ret)
        {
          LOG.Error("RUN READY precondition failed");
          m_threadPauser.ReportThreadPauserStatus(Logger::ERROR);
        }
        return ret;
      }

      virtual bool EvaluatePredicate()
      {
        return m_threadPauser.m_threadState == THREADSTATE_RUNNING;
      }
    } m_stateIsRunning;

  public:

    ThreadState GetStateBlockingInner()
    {
      return GetAdvanceStateInner(false);
    }

    ThreadState AdvanceStateInner()
    {
      return GetAdvanceStateInner(true);
    }

    ThreadState GetAdvanceStateInner(bool innerReadyToAdvance) ;

    ThreadState GetStateNonblocking() ;

    /**
     * Given the ThreadPauser is currently in state fromState, advance
     * it, if necessary, to the next state in the sequence.
     *
     * Violates an assertion if the ThreadPauser's current state is
     * not fromState.
     *
     * \returns the (now) current state of the ThreadPauser.
     */
    ThreadState AdvanceStateOuter(ThreadState fromState) ;

    /**
     * Constructs a new ThreadPauser.
     */
    ThreadPauser();

    /**
     * Destroys this ThreadPauser.
     */
    ~ThreadPauser();

    /**
     * To be called by the outer thread. This advances the inner
     * thread from THREADSTATE_RUNNING to THREADSTATE_PAUSE_REQUESTED
     */
    void RequestPause()
    {
      AdvanceStateOuter(THREADSTATE_RUNNING);
    }

    /**
     * Sets whether or not this ThreadPauser is allowed to ignore
     * problems consistent with threading bugs. This is not normally
     * reccomended because it will cause some corruptions, but it may
     * keep your MFM instance from crashing.
     */
    void SetIgnoreThreadingProblems(bool value)
    {
      m_ignoreThreadingProblems = value;
    }

    /**
     * To be called by the outer thread. This advances the inner
     * thread from THREADSTATE_PAUSED to THREADSTATE_RUN_REQUESTED
     */
    void RequestRun()
    {
      AdvanceStateOuter(THREADSTATE_PAUSED);
    }

    /**
     * To be called by the outer thread. This advances the inner
     * thread from THREADSTATE_RUN_READY to THREADSTATE_RUNNING
     */
    void Run()
    {
      AdvanceStateOuter(THREADSTATE_RUN_READY);
    }

    /**
     * To be called by the outer thread. Checks to see if the inner
     * thread is ready to be run.
     */
    bool IsRunReady()
    {
      return GetStateNonblocking() == THREADSTATE_RUN_READY;
    }

    /**
     * To be called by the outer thread. Checks to see if the inner
     * thread is ready to be paused.
     */
    bool IsPauseReady()
    {
      return GetStateNonblocking() == THREADSTATE_PAUSE_READY;
    }

    /**
     * To be called by the outer thread. ThreadPauser must be in state
     * THREADSTATE_PAUSE_READY. This returns immediately and pauses
     * the looping thread.
     */
    void Pause()
    {
      AdvanceStateOuter(THREADSTATE_PAUSE_READY);
    }

    static const char * GetThreadStateName(ThreadState ts) ;

    void ReportThreadPauserStatus(Logger::Level level) ;

#if 0
    /**
     * Finds out if an outside thread wishes this ThreadPauser to be
     * paused.
     */
    bool IsPauseRequested()
    {
      return GetStateNonblocking() == THREADSTATE_PAUSE_REQUESTED;
    }


    /**
     * Finds out if this ThreadPauser is actively trying to pause the
     * looping thread.
     */
    bool IsPaused();

    /**
     * Sets this ThreadPauser to be in a state which describes that it
     * is ready to be paused.
     */
    void PauseReady();

    /**
     * To be called by the looping thread. This pauses the thread
     * until its state is no longer paused.  ThreadPauser must be in
     * state THREAD_STATE_PAUSED.
     */
    void WaitWhilePaused();

#endif
  };
}

#endif /* THREAD_PAUSER_H */
