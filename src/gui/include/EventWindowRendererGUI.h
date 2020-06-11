/*                                              -*- mode:C++ -*-
  EventWindowRendererGUI.h Event window graphics functionality
  Copyright (C) 2020 The Regents of the University of New Mexico.  All rights reserved.

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
  \file EventWindowRendererGUI.h Event window graphics functionality
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef EVENTWINDOWRENDERERGUI_H
#define EVENTWINDOWRENDERERGUI_H

#include "EventWindowRenderer.h"

namespace MFM
{
  /**
     An EventWindowRendererGUI potentially provides access to a graphics
     drawing mechanism available to UlamClasses.  
   */
  template <class EC>
  class EventWindowRendererGUI : public EventWindowRenderer<EC>
  {
    class Drawing; // FORWARD
  private:
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };
  public:
    virtual Drawing * getDrawingOrNull() const { return 0; }
  };
} /* namespace MFM */

#include "EventWindowRendererGUI.tcc"

#endif /*EVENTWINDOWRENDERERGUI_H*/
