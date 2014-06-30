/*                                              -*- mode:C++ -*-
  AbstractHeadlessDriver.h Base class for all command-line MFM drivers
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
  \file AbstractHeadlessDriver.h Base class for all command-line MFM drivers
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACT_HEADLESS_DRIVER_H
#define ABSTRACT_HEADLESS_DRIVER_H

#include "Logger.h"
#include "FileByteSink.h"
#include "itype.h"
#include "ParamConfig.h"
#include "GridConfig.h"
#include "CoreConfig.h"
#include "Tile.h"
#include "Grid.h"
#include "ElementTable.h"
#include "Element_Empty.h" /* Need common elements */
#include "VArguments.h"
#include "AbstractDriver.h"

namespace MFM
{
  /**
   * A class representing a headless driver, i.e. a driver which works
   * only on the command line without input.
   */
  template<class GC>
  class AbstractHeadlessDriver : public AbstractDriver<GC>
  {
  private: typedef AbstractDriver<GC> Super;

  protected:
    typedef typename Super::OurGrid OurGrid;
    typedef typename Super::CC CC;

    AbstractHeadlessDriver() : AbstractDriver<GC>()
    { }

    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
    }

    virtual void PostUpdate()
    {
      LOG.Debug("AEPS: %d", (u32)Super::GetAEPS());
    }
  };
}

#endif /* ABSTRACT_HEADESS_DRIVER_H */
