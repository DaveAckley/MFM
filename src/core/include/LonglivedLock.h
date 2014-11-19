/*  -*- mode:C++ -*- */
/*
  LonglivedLock.h Mutual exclusion for long-term assets
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
  \file LonglivedLock.h Mutual exclusion for long-term assets
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef LONGLIVEDLOCK_H
#define LONGLIVEDLOCK_H

#include "itype.h"
#include "Fail.h"
#include "Mutex.h"
#include "Logger.h"

namespace MFM
{
  /**
   * An LonglivedLock mediates long-duration locking between a set of
   * possible owners
   */
  class LonglivedLock
  {
  private:
    Mutex m_shortLivedLock;
    void * m_longlivedLockOwner;
    void * m_lastLonglivedLockOwner;

    enum ThreeWayResult { RESULT_TRUE, RESULT_FALSE, RESULT_FAIL };

    bool DecodeResult(ThreeWayResult res)
    {
      switch(res)
      {
      case RESULT_TRUE: return true;
      case RESULT_FALSE:  return false;

      default:
      case RESULT_FAIL:
        FAIL(LOCK_FAILURE);
      }
    }

    ThreeWayResult TryLockInternal(void * arg)
    {
      Mutex::ScopeLock scopeLock(m_shortLivedLock);

      if (m_longlivedLockOwner == 0)
      {
        m_longlivedLockOwner = arg;
        m_lastLonglivedLockOwner = m_longlivedLockOwner;
        return RESULT_TRUE;
      }

      if (m_longlivedLockOwner == arg)
      {
        return RESULT_FAIL;
      }

      return RESULT_FALSE;
    }

    ThreeWayResult UnlockInternal(void * arg)
    {
      Mutex::ScopeLock scopeLock(m_shortLivedLock);

      if (m_longlivedLockOwner == arg)
      {
        m_longlivedLockOwner = 0;
        return RESULT_TRUE;
      }

      return RESULT_FAIL;
    }

  public:

    /**
     * Initialize this LonglivedLock.
     */
    LonglivedLock()
      : m_longlivedLockOwner(0)
    { }

    /**
     * Destroys this LonglivedLock.
     */
    ~LonglivedLock()
    { }

    /**
     * Get the current lock owner if any.  The result is only advisory
     * (i.e., it may have changed by the time caller looks at it)
     * unless the caller owns the channel.
     *
     * \returns 0 if the lock was free, or non-zero for owner index
     */
    void * GetOwnerIndex()
    {
      Mutex::ScopeLock scopeLock(m_shortLivedLock);
      return m_longlivedLockOwner;
    }

    /**
     * Atomically attempt to capture the lock on behalf of ownerIndex.
     * If the lock is available, change its ownder to ownerIndex and
     * return true.  If the lock is currently held by who, FAILS with
     * LOCK_FAILURE to discourage stupidity.  If the lock held by some
     * other owner index, change nothing and return false.
     */
    bool TryLock(void * who)
    {
      MFM_API_ASSERT_NONNULL(who);
      MFM_API_ASSERT(who != (void*) (intptr_t) -1,ILLEGAL_ARGUMENT);
      return DecodeResult(TryLockInternal(who));
    }

    /**
     * Atomically check and possibly update the long-lived lock as
     * follows: If the lock is currently held by ownerIndex, unlock
     * the long-lived lock and return true.  Otherwise do not change
     * the channel state and return false.
     */
    bool Unlock(void * who)
    {
      MFM_API_ASSERT_NONNULL(who);
      MFM_API_ASSERT(who != (void*) (intptr_t) -1,ILLEGAL_ARGUMENT);
      return DecodeResult(UnlockInternal(who));
    }
  };
}

#endif /* LONGLIVEDLOCK_H */
