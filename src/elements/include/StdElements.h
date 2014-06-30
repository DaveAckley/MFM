/* -*- mode:C++ -*- */

/*
  StdElements.h - A class to create references to all the distribution standard elements
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
   \file StdElements.h - A class to create references to all the distribution standard elements
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef STDELEMENTS_H
#define STDELEMENTS_H

#include "itype.h"
#include "CoreConfig.h"

namespace MFM {

  template <class CC>
  class StdElements {
  private:
    u32 m_stdElements;
  public:
    StdElements() ;
    u32 GetStdElementCount() { return m_stdElements; }
  };
} /* namespace MFM */

#include "StdElements.tcc"

#endif /*STDELEMENTS_H*/

