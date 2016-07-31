/*                                              -*- mode:C++ -*-
  UlamContext.h Access to the environment surrounding executing ulam code
  Copyright (C) 2014-2016 The Regents of the University of New Mexico.  All rights reserved.
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
  \file UlamContext.h Access to the environment surrounding executing ulam code
  \author David H. Ackley
  \author Elena S. Ackley
  \date (C) 2014-2016 All rights reserved.
  \lgpl
 */

#ifndef ULAMCONTEXT_H
#define ULAMCONTEXT_H

#include "itype.h"
#include "Fail.h"
#include "Element.h"

namespace MFM
{
  template <class AC> class Base; // FORWARD
  template <class AC> class Site; // FORWARD
  template <class EC> class EventWindow; // FORWARD
  template <class EC> class UlamClass; //FORWARD
  template <class EC> class UlamClassRegistry; //FORWARD
  template <class EC> class ElementTable; //FORWARD

  class Random; // FORWARD

  template <class EC>
  class UlamContext {

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;
    const ElementTable<EC>& m_elementTable;

  public:

    UlamContext(const ElementTable<EC>& et)
      : m_elementTable(et)
    { }

    UlamContext(const UlamContext<EC>& cxref);

    virtual ~UlamContext() { }

    virtual bool HasRandom() const { return false; }
    virtual Random & GetRandom() { FAIL(UNSUPPORTED_OPERATION); }

    virtual bool HasEventWindow() const { return false; }
    virtual EventWindow<EC> & GetEventWindow() { FAIL(UNSUPPORTED_OPERATION); }

    virtual const EventWindow<EC> & GetEventWindow() const { FAIL(UNSUPPORTED_OPERATION); }

    virtual bool HasUlamClassRegistry() const { return false; }
    virtual const UlamClassRegistry<EC> & GetUlamClassRegistry() const { FAIL(UNSUPPORTED_OPERATION); }

    virtual const Element<EC> * LookupElementTypeFromContext(u32 etype) const ;

    virtual const char * GetContextLabel() const { return "Generic UlamContext"; }

    const UlamClass<EC> * LookupUlamElementTypeFromContext(u32 etype) const ;

  };

} //MFM

#include "UlamContext.tcc"

#endif /* ULAMCONTEXT_H */
