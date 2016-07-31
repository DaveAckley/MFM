/*                                              -*- mode:C++ -*-
  DebugTools.h Miscellaneous gdb-level helper functions
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
  \file DebugTools.h Miscellaneous gdb-level helper functions
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \lgpl
 */
#ifndef DEBUGTOOLS_H
#define DEBUGTOOLS_H

#include "UlamContext.h"
#include "UlamRef.h"
#include "FileByteSink.h"

namespace MFM {

  template<class EC> 
  void DebugPrint(const UlamContext<EC>& uc, const UlamRef<EC>& ur, ByteSink& out) ;

  template<class EC>
  void DebugPrint(const UlamContext<EC>& uc, const UlamRef<EC>& ur) 
  {
    DebugPrint(uc, ur, STDERR);
  }

  template<class EC> 
  void DebugPrint(const UlamContext<EC>& uc, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom, ByteSink& out) ;

  template<class EC> 
  void DebugPrint(const UlamContext<EC>& uc, const AtomBitStorage<EC>& abs, ByteSink& out)
  {
    const typename EC::ATOM_CONFIG::ATOM_TYPE atom = abs.ReadAtom();
    DebugPrint(uc, atom, out);
  }

  template<class EC>
  void InitDebugTools();
}

#include "DebugTools.tcc"

#endif /* DEBUGTOOLS_H */
