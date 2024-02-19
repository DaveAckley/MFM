/*                                              -*- mode:C++ -*-
  V6Panel.h Panel redo attempt with resizing and content flowing
  Copyright (C) 2023 Living Computation Foundation

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
  \file V6Panel.h Panel redo with resizing and content flowing
  \author David H. Ackley.
  \date (C) 2023 All rights reserved.
  \lgpl
 */
#ifndef V6PANEL_H
#define V6PANEL_H

#include "CloseWindowButton.h"
#include "MovablePanel.h"

namespace MFM
{
  class V6Panel : public MovablePanel
  {
    typedef MovablePanel Super;
    
   private:

    template <unsigned COUNT> friend class V6PanelPool;  // Let the pool mess with our index
    s32 m_poolIndex;

    static const u32 cBG_COLOR = 0x102040;
    static const u32 cFG_COLOR = 0xffdfbf;

   public:
    V6Panel()
      : MovablePanel(300, 100)
      , m_poolIndex(-1)
    { }

    void Init(const char * owner, unsigned i) ;

    virtual bool Handle(KeyboardEvent & event) ;

    virtual bool Handle(MouseButtonEvent & event) ;

    virtual bool Handle(MouseMotionEvent & event) ;

    virtual void PaintBorder(Drawing & d) ;

    virtual void PaintComponent(Drawing& d) ;

  };
}

#endif /* V6PANEL_H */
