/*                                              -*- mode:C++ -*-
  Configurable.h Abstract class for externally configuring signed integral parameters
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
  \file Configurable.h Abstract class for externally configuring signed integral parameters
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H

#include "OverflowableCharBufferByteSink.h"

namespace MFM
{
  class Configurable
  {
   public:
    virtual u32 GetConfigurableCount()
    {
      return 0;
    }

    virtual s32* GetConfigurableParameter(u32 index)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

    virtual s32 GetMinimumValue(u32 index)
    {
      return 1;
    }

    virtual s32 GetMaximumValue(u32 index)
    {
      return 1000;
    }

    /**
     * Gets the snapping resolution of a parameter. This is used to
     * snap a slider to a particular number of values. If not
     * specified, the default is one.
     */
    virtual u32 GetSnapResolution(u32 index)
    {
      return 1;
    }

    virtual const char* GetConfigurableName(u32 index)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
  };
}

#endif /* CONFIGURABLE_H */
