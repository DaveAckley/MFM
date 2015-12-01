/*                                              -*- mode:C++ -*-
  UlamClassRegistry.h A class for the registration of ULAM quarks and elements
  Copyright (C) 2015 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015 Ackleyshack LLC.

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
  \file UlamClassRegistry.h A class for the registration of ULAM quarks and elements
  \author David H. Ackley.
  \author Elenas S. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */

#ifndef ULAMCLASSREGISTRY_H
#define ULAMCLASSREGISTRY_H

#include "itype.h"

namespace MFM {

  struct UlamClass; //FORWARD

  struct UlamClassRegistry {
    enum {
      TABLE_SIZE = 100
    };

    bool RegisterUlamClass(UlamClass& uc) ;

    s32 GetUlamClassIndex(const char *) const;

    bool IsRegisteredUlamClass(const char *mangledName) const;

    const UlamClass * GetUlamClassByMangledName(const char *mangledName) const;

    const UlamClass * GetUlamClassByIndex(u32 index) const;

    UlamClass * m_registeredUlamClasses[TABLE_SIZE];
    u32 m_registeredUlamClassCount;
  };

} //MFM
#include "UlamClassRegistry.tcc"

#endif /* ULAMCLASSREGISTRY_H */
