/*                                              -*- mode:C++ -*-
  UlamContext.h Access to the environment surrounding executing ulam code
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.
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
  \file UlamContext.h Access to the environment surrounding executing ulam code
  \author David H. Ackley
  \author Elena S. Ackley
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */

#ifndef ULAMCONTEXT_H
#define ULAMCONTEXT_H

#include "itype.h"

namespace MFM
{
  template <class AC> class Base; // FORWARD
  template <class AC> class Site; // FORWARD
  template <class EC> class Tile; // FORWARD
  template <class EC> class EventWindow; // FORWARD
  template <class EC> class UlamClass; //FORWARD

  class Random; // FORWARD

  template <class EC>
  class UlamContext {

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;

    mutable Tile<EC> * m_tile;
    void AssertTile() const;

  public:

    UlamContext();
    //    UlamContext(const UlamContext<EC>& cxref , const UlamClass<EC> * ucp);
    UlamContext(const UlamContext<EC>& cxref);

    void SetTile(Tile<EC> & t) ;

    Tile<EC> & GetTile() ;

    const Tile<EC> & GetTile() const ;

    Random & GetRandom() ;

    EventWindow<EC> & GetEventWindow() ;

    const EventWindow<EC> & GetEventWindow() const ;

    Base<AC> & GetBase() ;

    const Site<AC> & GetSite() const ;

    const UlamClass<EC> * LookupElementTypeFromContext(u32 etype) const ;
  };

} //MFM

#include "UlamContext.tcc"

#endif /* ULAMCONTEXT_H */
