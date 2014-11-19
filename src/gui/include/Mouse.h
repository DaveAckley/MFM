/*                                              -*- mode:C++ -*-
  Mouse.h Mouse input tracking system
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
  \file Mouse.h Mouse input tracking system
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef MOUSE_H
#define MOUSE_H

#include <set>
#include "itype.h"
#include "Point.h"
#include "SDL.h"

namespace MFM {

  class Mouse
  {
  private:
    std::set<u8> m_current;
    std::set<u8> m_prev;

    u16 m_x, m_y;

  public:

    Mouse() { }

    ~Mouse() { }

    void HandleButtonEvent(SDL_MouseButtonEvent* e);

    void HandleMotionEvent(SDL_MouseMotionEvent* e);

    void Press(u8 button);

    void Release(u8 button);

    bool IsDown(u8 button);

    bool IsUp(u8 button);

    bool SemiAuto(u8 button);

    u16 GetX() { return m_x; }

    u16 GetY() { return m_y; }

    void FillPoint(SPoint* out);

    /*
     * This should be called once a
     * frame to keep the SemiAuto
     * function working.
     */
    void Flip();
  };
} /* namespace MFM */
#endif /*MOUSE_H*/
