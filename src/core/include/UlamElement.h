/*                                              -*- mode:C++ -*-
  UlamElement.h A concrete base class for ULAM elements
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
  \file UlamElement.h A concrete base class for ULAM elements
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef DEFAULTELEMENT_H
#define DEFAULTELEMENT_H

#include "Element.h"
#include "Tile.h"
#include "EventWindow.h"
#include "UlamContext.h"

namespace MFM
{
  /**
   * A UlamElement is a concrete element, primarily for use by culam.
   */
  template <class CC>
  class UlamElement : public Element<CC>
  {
    typedef typename CC::ATOM_TYPE T;

  public:
    UlamElement(const UUID & uuid) : Element<CC>(uuid)
    { }

    /**
     * Destroys this UlamElement.
     */
    virtual ~UlamElement()
    { }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Tile<CC> & tile = window.GetTile();
      UlamContext<CC> & uc = UlamContext<CC>::Get();
      uc.SetTile(tile);
      T me = window.GetCenterAtom();
      T orig = me;
      Uf_6behave(me);
      T post = window.GetCenterAtom();
      if ( me != orig && orig == post) // If I've changed but not the world
        window.SetCenterAtom(me);     // write me back.
    }

    /**
       Ulam elements that define 'Void behave()' will override this
       method, and it will be called on events!
     */
    virtual void Uf_6behave(T& self) const
    {
      // Empty by default
    }

    virtual const UlamElement<CC> * AsUlamElement() const
    {
      return this;
    }

    /**
       Find the first position of a data member in this element by the
       name of its type, if any exist.

       \return The smallest bit position of an occurrence of the type
               \c dataMemberTypeName in this element, if any, or -1 to
               indicate the given type is not used as a data member in
               this UlamElement.  A return value of 0 corresponds to
               the ATOM_FIRST_STATE_BIT of the atom.

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual s32 PositionOfDataMemberType(const char * dataMemberTypeName) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    virtual u32 DefaultPhysicsColor() const
    {
      // Colored white by default
      return 0xffffffff;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }


  };
}

#endif /* DEFAULTELEMENT_H */
