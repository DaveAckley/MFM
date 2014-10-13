/*                                              -*- mode:C++ -*-
  StdPanel.h Extensions for the Panel structure
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
  \file StdPanel.h Extensions for the Panel structure
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef STDPANEL_H
#define STDPANEL_H

#include "itype.h"
#include "Panel.h"
#include "SDL.h"

namespace MFM {

  /**
     StdPanel adds several common behaviors to a Panel.
   */
  class StdPanel : public Panel
  {
  private:

    SPoint m_dragStart;
    bool m_dragging;

  public:

    StdPanel() : m_dragging(false) { }

    virtual bool Handle(SDL_MouseButtonEvent & event) ;

    virtual bool Handle(SDL_MouseMotionEvent & event) ;

    virtual bool Click(const SPoint & position, u32 button, u32 modifiers) {
      return false;
    }

    virtual bool ScrollUp() {
      return false;
    }

    virtual bool ScrollDown() {
      return false;
    }

    virtual void DragStart(const SPoint & startPosition) {
      /* empty */
    }

    virtual void Drag(const SPoint & relativeDelta) {
      /* empty */
    }

    virtual void DragEnd(const SPoint & endPosition) {
      /* empty */
    }

  };
} /* namespace MFM */
#endif /*STDPANEL_H*/
