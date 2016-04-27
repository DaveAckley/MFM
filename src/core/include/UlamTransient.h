/*                                              -*- mode:C++ -*-
  UlamTransient.h A concrete base class for ULAM transients
  Copyright (C) 2016 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2016 Ackleyshack LLC.

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
  \file UlamTransient.h A concrete base class for ULAM transient
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2016 All rights reserved.
  \lgpl
*/
#ifndef ULAMTRANSIENT_H
#define ULAMTRANSIENT_H

#include "UlamClass.h"
#include "UUID.h"
#include "UlamRef.h"

namespace MFM {
  /**
   * A UlamTransient is a concrete transient primarily for use by culam.
   */
  template <class EC>
  class UlamTransient : public UlamClass<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

  public:

    UlamTransient(const UUID & uuid) : UlamClass<EC>() { }

    virtual ~UlamTransient() { }

    virtual bool IsUlamTransient() const
    {
      return true;
    }

    virtual void getDefaultTransient(u32 pos, BitStorage<EC> & bvsref) const = 0;

  private:
  };

} //MFM

#include "UlamTransient.tcc"

#endif /* ULAMTRANSIENT_H */
