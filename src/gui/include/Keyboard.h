/*                                              -*- mode:C++ -*-
  Keyboard.h Keyboard input tracking system
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
  \file Keyboard.h Keyboard input tracking system
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <set>
#include "itype.h"
#include "SDL.h"

namespace MFM {

  class Keyboard
  {
  private:
    std::set<u32> m_current;
    std::set<u32> m_prev;

  public:

    Keyboard() { }

    ~Keyboard() { }

    void HandleEvent(SDL_KeyboardEvent* e);

    void Press(u32 key);

    void Release(u32 key);

    bool ShiftHeld() const;

    bool CtrlHeld() const;

    bool AltHeld() const;

    bool IsDown(u32 key) const;

    bool IsUp(u32 key) const;

    bool SemiAuto(u32 key);

    void Flip();
  };
} /* namespace MFM */
#endif /*KEYBOARD_H*/
