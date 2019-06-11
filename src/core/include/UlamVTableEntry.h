/*                                              -*- mode:C++ -*-
  UlamVTableEntry.h An abstract base class for ULAM vtable entry
  Copyright (C) 2019 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2019 Ackleyshack LLC.

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
  \file UlamVTableEntry.h An abstract base class for ULAM vtable entry
  \author Elenas S. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef ULAMVTABLEENTRY_H
#define ULAMVTABLEENTRY_H

#include "itype.h"

template <class EC> class UlamClass; //FORWARD

namespace MFM
{
  typedef void (*VfuncPtr)(); // Generic function pointer we'll cast at point of use

  template<class EC>
  struct VTentry {
    VfuncPtr vfptr;
    //u16 oclassrelpos;
    //u16 oclasslen;
    //u32 oclassregnum;
    UlamClass<EC> * oclassptr;
  };

} //MFM

#endif /* ULAMVTABLEENTRY_H */
