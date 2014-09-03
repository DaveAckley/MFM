/*                                              -*- mode:C++ -*-
  Element_Template.h Tutorial template for a new Element
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

/* <<TEMPLATE>> Replace doxygen and license information with your element information. */
/**
  \file   Element_Template.h Tutorial template for a new Element
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_TEMPLATE_H /* <<TEMPLATE>> Replace header guard with your element name.*/
#define ELEMENT_TEMPLATE_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"

namespace MFM
{

  #define TEMPLATE_VERSION 1

  template <class CC>
  class Element_Template : public Element<CC> /* <<TEMPLATE>> Replace class name with yours */
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:

    ElementParameterS32<CC> m_sampleParameter;

    /* <<TEMPLATE>> Add any configurable parameters here. */

  public:

    /* <<TEMPLATE>> Replace class name with yours. Don't forget the '<CC>'. */
    static Element_Template<CC> THE_INSTANCE;

    Element_Template()
      : Element<CC>(MFM_UUID_FOR("Template", TEMPLATE_VERSION)),
        /* <<TEMPLATE>> Initialize all configurable parameters here. */
        m_sampleParameter(this, "sample", "Sample Parameter",
                  "This is the description for a sample parameter.", 1, 200, 1000, 10)
    {
      /* <<TEMPLATE>> Set atomic symbol and name for your element. */
      Element<CC>::SetAtomicSymbol("Tm");
      Element<CC>::SetName("Template");
    }

    /*
       <<TEMPLATE>> Set how likely your element is to be moved by another element. See
                    Element.h for details.
    */
    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    /* <<TEMPLATE>> This color will be the default rendering color for your element. */
    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffff00ff;
    }

    /*
       <<TEMPLATE>> This color will be the color rendered when your
                    element is rendered in lowlighting.
     */
    virtual u32 DefaultLowlightColor() const
    {
      return 0xff770077;
    }

    /*
      <<TEMPLATE>> This is a short description of your element.
     */
    virtual const char* GetDescription() const
    {
      return "Short for \"Dynamic Regulator\", This Atom controls the density of "
             "nearby Atoms by creating RES atoms and deleting nearby atoms.";
    }

    /*
      <<TEMPLATE>> This method is executed every time an atom of your
                   element is chosen for an event. See the tutorial in
                   the wiki for further information.
     */
    virtual void Behavior(EventWindow<CC>& window) const
    {
    }
  };

  /*
     <<TEMPLATE>> Rename the class names here to the class name of your element.
  */
  template <class CC>
  Element_Template<CC> Element_Template<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_TEMPLATE_H */ /* <<TEMPLATE>> Rename the header guard to your element */
