/*                                              -*- mode:C++ -*-
  AbstractLockSet.h Manage a Tile's worth of AbstractLocks
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
  \file AbstractLockSet.h Manage a Tile's worth of AbstractLocks
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \lgpl
 */
#ifndef LOCKSET_H
#define LOCKSET_H

#include "AbstractLock.h"

namespace MFM {

  typedef u32 LockNumber;

  class AbstractLockSet 
  {
    static const LockNumber EAST_LOCK = 0;
    static const LockNumber MIDDLE_LOCK = EAST_LOCK + 1;
    static const LockNumber SOUTH_LOCK = MIDDLE_LOCK + 1;
    static const LockNumber LOCK_COUNT = SOUTH_LOCK + 1;

  public:
    /**
       Access the AbstractLock associated with lock number \c ln.
       FAILs with ILLEGAL_ARGUMENT if \c ln is not one of EAST_LOCK or
       MIDDLE_LOCK or SOUTH_LOCK.
     */
    virtual AbstractLock & GetLock(LockNumber ln) = 0;
  };

}

#endif /* LOCKSET_H */
