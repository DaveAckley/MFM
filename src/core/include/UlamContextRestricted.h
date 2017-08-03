/*                                              -*- mode:C++ -*-
  UlamContextRestricted.h Restricted access to the environment for ulam code executing outside an event
  Copyright (C) 2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2017 Ackleyshack LLC.

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
  \file UlamContextRestricted.h Restricted access to the environment for ulam code executing outside an event
  \author David H. Ackley
  \author Elena S. Ackley
  \date (C) 2017 All rights reserved.
  \lgpl
 */

#ifndef ULAMCONTEXTRESTRICTED_H
#define ULAMCONTEXTRESTRICTED_H

#include "UlamContext.h"
#include "UlamClassRegistry.h"

namespace MFM
{
  template <class EC>
  class UlamContextRestricted : public UlamContext<EC> {

    const UlamClassRegistry<EC> & m_ucr;
    void AssertTile() const;

  public:

    UlamContextRestricted(const ElementTable<EC>& et, const UlamClassRegistry<EC>& ucr) 
      : UlamContext<EC>(et)
      , m_ucr(ucr)
    { }

    UlamContextRestricted(const UlamContextRestricted<EC>& cxref);

    virtual bool HasRandom() const { return false; }
    virtual Random & GetRandom() { FAIL(UNSUPPORTED_OPERATION); }

    virtual bool HasEventWindow() const { return false; }
    virtual EventWindow<EC> & GetEventWindow() { FAIL(UNSUPPORTED_OPERATION); }
    virtual const EventWindow<EC> & GetEventWindow() const { FAIL(UNSUPPORTED_OPERATION); }

    virtual bool HasUlamClassRegistry() const { return true; }
    virtual const UlamClassRegistry<EC> & GetUlamClassRegistry() const { return m_ucr; }

    virtual const char * GetContextLabel() const { return "UlamContextRestricted without Tile"; }
  };

}

#include "UlamContextRestricted.tcc"

#endif /* ULAMCONTEXTRESTRICTED_H */
