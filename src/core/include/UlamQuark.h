/*                                              -*- mode:C++ -*-
  UlamQuark.h A concrete base class for ULAM quarks
  Copyright (C) 2015-2016 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015-2016 Ackleyshack LLC.

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
  \file UlamQuark.h A concrete base class for ULAM quark
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2015-2016 All rights reserved.
  \lgpl
 */
#ifndef ULAMQUARK_H
#define ULAMQUARK_H

#include "UlamClass.h"

namespace MFM {

  /**
   * A UlamQuark is a concrete quark primarily for use by culam.
   */
  template <class EC>
  class UlamQuark : public UlamClass<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

  public:

    UlamQuark(const UUID & uuid) : UlamClass<EC>() { }

    virtual ~UlamQuark() { }

    virtual UlamQuark<EC>* AsUlamQuark()
    {
      return this;
    }

    virtual const UlamQuark<EC> * AsUlamQuark() const
    {
      return this;
    }

    virtual u32 getDefaultQuark() const = 0;
  };

} //MFM

#include "UlamQuark.tcc"

#endif /* ULAMQUARK_H */
