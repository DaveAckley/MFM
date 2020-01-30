/*                                              -*- mode:C++ -*-
  EventWindowRenderer.h Core stub for event window graphics functionality
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
  \file EventWindowRenderer.h Core stub for event window graphics functionality
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef EVENTWINDOWRENDERER_H
#define EVENTWINDOWRENDERER_H
#include "Drawable.h"

namespace MFM
{
  /**
     An EventWindowRenderer potentially provides access to a graphics
     drawing mechanism for UlamClasses.
   */
  template <class EC>
  class EventWindowRenderer
  {
  private:
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };
  public:
    virtual Drawable * getDrawableOrNull() const { return 0; }
  };
} /* namespace MFM */

#include "EventWindowRenderer.tcc"

#endif /*EVENTWINDOWRENDERER_H*/
