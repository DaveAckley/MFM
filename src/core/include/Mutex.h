/*                                              -*- mode:C++ -*-
  Mutex.h A wrapper class for dealing with pthread_mutexes
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
  \file Mutex.h A wrapper class for dealing with pthread_mutexes
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>  /* for pthread_mutex_t etc */
#include <errno.h>    /* for EBUSY */
#include "itype.h"
#include "Fail.h"

#ifdef MUTEX_ERROR_CHECKS
#define MFM_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK
#else
#define MFM_MUTEX_TYPE PTHREAD_MUTEX_NORMAL
#endif

namespace MFM
{
  class Mutex
  {
   private:
    /**
     * The underlying pthread_mutex_t used by this Mutex
     */
    pthread_mutex_t m_lock;

    pthread_mutexattr_t m_attr;

    /**
     * True if currently locked
     */
    bool m_locked;

    /**
     * If the lock is held, the pthread id of the thread holding it.
     */
    pthread_t m_threadId;

    // Declare away copy ctor; pthread_mutexes can't be copied
    Mutex(const Mutex & ) ;

    void CondWait(pthread_cond_t & condvar)
    {
      MFM_API_ASSERT(!pthread_cond_wait(&condvar, &m_lock), LOCK_FAILURE);

      // The signal gave us back the lock without going through
      // Mutex::Lock; simulate its effects
      MFM_API_ASSERT(!m_locked, LOCK_FAILURE);

      m_threadId = pthread_self();
      m_locked = true;
    }

   public:

    class ScopeLock
    {
     private:
      Mutex & m_mutex;
     public:
      ScopeLock(Mutex & mutex) : m_mutex(mutex)
      {
        m_mutex.Lock();
      }
      ~ScopeLock()
      {
        m_mutex.Unlock();
      }
    };

    class Predicate
    {
     private:
      Mutex & m_mutex;
      pthread_cond_t m_condvar;
      pthread_t m_threadIdOfWaiter;
      u32 m_wakeupsThisWait;

      Predicate() ; // Declare away
      Predicate(const Predicate &) ; // Declare away
      Predicate & operator=(const Predicate &) ; // Declare away
    public:

      virtual bool EvaluatePrecondition() = 0;

      virtual bool EvaluatePredicate() = 0;

      Predicate(Mutex & mutex) : m_mutex(mutex)
      {
        MFM_API_ASSERT(!pthread_cond_init(&m_condvar, NULL), LOCK_FAILURE);
      }

      ~Predicate()
      {
        MFM_API_ASSERT(!pthread_cond_destroy(&m_condvar), LOCK_FAILURE);
      }

      u32 GetWakeupsThisWait() const
      {
        return m_wakeupsThisWait;
      }

      void WaitForCondition()
      {
        m_mutex.AssertIHoldTheLock();
        m_threadIdOfWaiter = m_mutex.m_threadId;
        m_wakeupsThisWait = 0;

        while (!EvaluatePredicate())
        {
          // If the predicate's not true, the precondition must be
          MFM_API_ASSERT(EvaluatePrecondition(), LOCK_FAILURE);
          m_mutex.CondWait(m_condvar);
          ++m_wakeupsThisWait;
        }
      }

      void SignalCondition()
      {
        m_mutex.AssertIHoldTheLock();
        MFM_API_ASSERT(!pthread_cond_signal(&m_condvar), LOCK_FAILURE);
      }
    };

    /**
     * Create a new unlocked Mutex
     *
     * FAILs with LOCK_FAILURE if the underlying pthread_mutex cannot
     * be initialized.
     */
    Mutex() :
      m_locked(false)
    {

      MFM_API_ASSERT(!pthread_mutexattr_init(&m_attr), LOCK_FAILURE);
      MFM_API_ASSERT(!pthread_mutexattr_settype(&m_attr, MFM_MUTEX_TYPE), LOCK_FAILURE);
      MFM_API_ASSERT(!pthread_mutex_init(&m_lock, &m_attr), LOCK_FAILURE);
    }

    /**
     * Destroy a Mutex
     *
     * FAILs with LOCK_FAILURE if:
     * - The Mutex is currently locked
     * - the underlying pthread_mutex_destroy fails.
     */
    ~Mutex()
    {
      MFM_API_ASSERT(!m_locked, LOCK_FAILURE);
      MFM_API_ASSERT(!pthread_mutex_destroy(&m_lock), LOCK_FAILURE);
      MFM_API_ASSERT(!pthread_mutexattr_destroy(&m_attr), LOCK_FAILURE);
    }

    /** Attempt to lock the underlying mutex, without blocking
     *
     * @returns true if the lock was successfully taken, else false.
     *
     * FAILs with LOCK_FAILURE if:
     * - the Mutex is already locked by the calling thread.
     */
    bool TryLock()
    {
      MFM_API_ASSERT(!m_locked || !pthread_equal(m_threadId, pthread_self()), LOCK_FAILURE);

      int status = pthread_mutex_trylock(&m_lock);

      if (status == 0) {
        m_threadId = pthread_self();
        m_locked = true;
        return true;
      }

      MFM_API_ASSERT(status == EBUSY, LOCK_FAILURE);
      return false;
    }

    /** Locks the underlying mutex, blocking as long as necessary to
     * acquire it.
     *
     * FAILs with LOCK_FAILURE if:
     * - The mutex is already locked by the current thread
     */
    void Lock()
    {
      int status = pthread_mutex_lock(&m_lock);
      MFM_API_ASSERT(status == 0, LOCK_FAILURE);
      MFM_API_ASSERT(!m_locked || !pthread_equal(m_threadId, pthread_self()), LOCK_FAILURE);

      // Update threadid before declaring locked so, for example, the
      // second condition in this method can't trigger on a possibly
      // stale threadid.
      m_threadId = pthread_self();
      m_locked = true;
    }

    /**
     * Confirm that the current thread holds the lock.
     *
     * FAILs with LOCK_FAILURE if:
     * - The mutex is currently not locked
     * - The mutex is locked but the current thread did not lock it
     */
    void AssertIHoldTheLock()
    {
      MFM_API_ASSERT(m_locked, LOCK_FAILURE);
      MFM_API_ASSERT(pthread_equal(m_threadId, pthread_self()), LOCK_FAILURE);
    }

    /**
     * Unlocks the underlying mutex.
     *
     * FAILs with LOCK_FAILURE if:
     * - The mutex is currently not locked
     * - The mutex is locked but the current thread did not lock it
     */
    void Unlock()
    {
      AssertIHoldTheLock();

      m_locked = false;
      m_threadId = 0;
      MFM_API_ASSERT(!pthread_mutex_unlock(&m_lock), LOCK_FAILURE);
    }

    /**
     * Test if the lock is held by a thread other than the current
     * thread.
     *
     * \return false if the mutex is currently unlocked; true if it is
     * locked by a thread other than the current thread.
     *
     * FAILs with LOCK_FAILURE if:
     * - The mutex is locked by the current thread
     */
    bool IsLockedByAnother()
    {
      if (!m_locked)
      {
        return false;
      }

      MFM_API_ASSERT(!pthread_equal(m_threadId, pthread_self()), LOCK_FAILURE);

      return true;
    }

    void ReportMutexStatus(int level) ;  // level not Logger::Level to avoid #include loop :(

#ifdef LOSER_LOCK
    /**
     * Check if the lock is held by you (loser routine).
     *
     * NOTE: If you need to call this routine then your code is
     * broken!  You must always know what locks you hold!
     */
    bool IHoldThisLock() const
    {
      return m_locked && pthread_equal(m_threadId, pthread_self());
    }
#endif /* LOSER_LOCK */

  };
} /* namespace MFM */

#endif /*MUTEX_H*/
