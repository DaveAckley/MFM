/*                                              -*- mode:C++ -*-
  UlamContextEvent.h Access to the environment surrounding executing ulam code
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
  \file UlamContextEvent.h Access to the environment surrounding executing ulam code
  \author David H. Ackley
  \author Elena S. Ackley
  \date (C) 2014-2016 All rights reserved.
  \lgpl
 */

#ifndef ULAMCONTEXTEVENT_H
#define ULAMCONTEXTEVENT_H

#include "UlamContext.h"

namespace MFM
{
  template <class EC>
  class UlamContextEvent : public UlamContext<EC> {

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;

    mutable Tile<EC> * m_tile;
    void AssertTile() const;

    Tile<EC> & GetTile() ;
    
    const Tile<EC> & GetTile() const ;

  public:

    UlamContextEvent(const ElementTable<EC>& et) 
      : UlamContext<EC>(et)
      , m_tile(0)
    { }

    UlamContextEvent(const UlamContextEvent<EC>& cxref);

    void SetTile(Tile<EC> & t) ;

    virtual bool HasRandom() const { return m_tile; }
    virtual Random & GetRandom() 
    {
      if (!m_tile) FAIL(UNSUPPORTED_OPERATION); 
      return m_tile->GetRandom();
    }

    virtual bool HasEventWindow() const { return m_tile; }
    virtual EventWindow<EC> & GetEventWindow() 
    { 
      if (!m_tile) FAIL(UNSUPPORTED_OPERATION); 
      return m_tile->GetEventWindow();
    }
    virtual const EventWindow<EC> & GetEventWindow() const 
    { 
      if (!m_tile) FAIL(UNSUPPORTED_OPERATION); 
      return m_tile->GetEventWindow();
    }

    virtual bool HasUlamClassRegistry() const { return m_tile; }
    virtual const UlamClassRegistry<EC> & GetUlamClassRegistry() const { 
      if (!m_tile) FAIL(UNSUPPORTED_OPERATION); 
      return m_tile->GetUlamClassRegistry();
    }

    virtual const char * GetContextLabel() const { 
      if (m_tile) return m_tile->GetLabel();
      return "UlamContextEvent without Tile"; 
    }



  };

}

#include "UlamContextEvent.tcc"

#endif /* ULAMCONTEXTEVENT_H */
