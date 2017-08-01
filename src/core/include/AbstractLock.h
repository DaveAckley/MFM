/*                                              -*- mode:C++ -*-
  AbstractLock.h Intertile locking interface
  Copyright (C) 2016 The Regents of the University of New Mexico.  All rights reserved.

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
  \file AbstractLock.h Intertile locking interface
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTLOCK_H
#define ABSTRACTLOCK_H

#include "itype.h"

namespace MFM
{
  /**
   * An AbstractLock represents some method of relatively long-lasting
   * mutual exclusion between a set of possible owners
   */
  class AbstractLock
  {

  public:
    /**
     * Destroys this AbstractLock.
     */
    virtual ~AbstractLock()
    { }

    /**
     * Get the current lock owner if any.  The result is only advisory
     * (i.e., it may have changed by the time caller looks at it)
     * unless the caller owns the channel.
     *
     * \returns 0 if the lock was free, or non-zero for owner index
     */
    virtual void * GetOwnerIndex() = 0;

    /**
     * Atomically attempt to capture the lock on behalf of \c who.  If
     * the lock is available, change its ownder to \c who and return
     * true.  If the lock is currently held by \c who, FAILS with
     * LOCK_FAILURE to discourage stupidity.  If the lock held by some
     * other owner index, change nothing and return false.
     */
    virtual bool TryLock(void * who) = 0;

    /**
     * Atomically check and possibly update the long-lived lock as
     * follows: If the lock is currently held by ownerIndex \c who,
     * unlock the long-lived lock and return true.  Otherwise do not
     * change the channel state and return false.
     */
    virtual bool Unlock(void * who) = 0;
  };
}

#endif /* ABSTRACTLOCK_H */
