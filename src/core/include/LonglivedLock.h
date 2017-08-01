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
#include "AbstractLock.h"

namespace MFM
{
  /**
   * An LonglivedLock mediates long-duration locking between a set of
   * possible owners
   */
  class LonglivedLock : public AbstractLock
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
       \copydoc AbstractLock::GetOwnerIndex
     */
    void * GetOwnerIndex()
    {
      Mutex::ScopeLock scopeLock(m_shortLivedLock);
      return m_longlivedLockOwner;
    }

    /**
       \copydoc AbstractLock::TryLock
     */
    bool TryLock(void * who)
    {
      MFM_API_ASSERT_NONNULL(who);
      MFM_API_ASSERT(who != (void*) (intptr_t) -1,ILLEGAL_ARGUMENT);
      return DecodeResult(TryLockInternal(who));
    }

    /**
       \copydoc AbstractLock::Unlock
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
