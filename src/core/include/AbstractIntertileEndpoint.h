/*                                              -*- mode:C++ -*-
  AbstractIntertileEndpoint.h A termination point for all intertile activity
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
  \file AbstractIntertileEndpoint.h A termination point for all intertile activity
  \author David H. Ackley
  \date (C) 2016 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTINTERTILEENDPOINT_H
#define ABSTRACTINTERTILEENDPOINT_H

#include "itype.h"
#include "Fail.h"
#include "Logger.h"
#include "Point.h"
#include "AbstractChannel.h"
#include "AbstractLock.h"

namespace MFM
{
  /**
    An interface describing one end of an intertile connection, which
    includes information about (1) locking, (2) channel I/O, and (3)
    the remote tile origin.
   */
  class AbstractIntertileEndpoint {

  public:
    void ReportAbstractIntertileEndpointStatus(Logger::Level level);

    virtual SPoint GetRemoteTileOrigin() const = 0;

    virtual AbstractChannel & GetAbstractChannel() const = 0;

    virtual AbstractLock & GetAbstractLock() const = 0;

    AbstractIntertileEndpoint()
    { }

    virtual ~AbstractIntertileEndpoint()
    { }
  };
}

#endif /* ABSTRACTINTERTILEENDPOINT_H */
