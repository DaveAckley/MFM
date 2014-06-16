/*                                              -*- mode:C++ -*-
  AbstractDualDriver.h Compilation switch for GUI enabling
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
  \file AbstractDualDriver.h Compilation switch for GUI enabling
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACT_DUAL_DRIVER_H
#define ABSTRACT_DUAL_DRIVER_H


#ifdef MFM_GUI_DRIVER

#include "AbstractGUIDriver.h"
#define DUAL_DRIVER_TYPE AbstractGUIDriver

#else

#include "AbstractHeadlessDriver.h"
#define DUAL_DRIVER_TYPE AbstractHeadlessDriver

#endif

namespace MFM
{
  /**
   * A quantum driver, existing both as an AbstractGUIDriver and an
   * AbstractHeadlessDriver. If the symbol MFM_GUI_DRIVER is set, this
   * will build as a GUI driver. If not, this will build as a headless
   * driver.
   */
  template<class GC>
  class AbstractDualDriver : public DUAL_DRIVER_TYPE<GC>
  {
  private: typedef DUAL_DRIVER_TYPE<GC> Super;

  protected:

    AbstractDualDriver()
    { }

    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
    }

  };
}

#endif /* ABSTRACT_DUAL_DRIVER_H */
