/*                                              -*- mode:C++ -*-
  RootPanel.h T2Viz root panel
  Copyright (C) 2019 The T2 Tile Project

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
  \file RootPanel.h T2Viz root panel
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef ROOTPANEL_H
#define ROOTPANEL_H

#include "itype.h"
#include "Panel.h"
#include "Drawing.h"
#include "Fail.h"
#include "SDL.h"

namespace MFM
{
  /**
   * The root panel for T2Viz
   */
  class RootPanel : public Panel
  {
    typedef Panel Super;

  private:

    /**
     * Initializes this RootPanel
     */
    void Init() { }

  public:

    RootPanel() { }

    /**
     * Destructor.
     */
    ~RootPanel() { }

    /**
     * Sets the location of this RootPanel, relative to the Panel
     * which it is located inside of, at which to render this
     * RootPanel and process mouse hits.
     *
     * @param location The new location that this RootPanel will
     *                 reside at.
     */
    void SetLocation(const SPoint& location)
    {
      SetRenderPoint(location);
    }

    virtual void HandleResize(const UPoint& parentSize)
    {
      /* RootPanels don't resize automatically? */
    }

#if 0
    /* Trying to handle mouse events in t2viz doesn't make sense --
       mfmt2 is (in general) still running, and it (races and) takes
       them.
    */
    bool Handle(MouseButtonEvent& mbe) 
    {
      SDL_Quit(); /* die on mouse click? */

      /*
      SDL_MouseButtonEvent & event = mbe.m_event.button;
      bool isLeft = event.button == SDL_BUTTON_LEFT;
      bool isRight = event.button == SDL_BUTTON_RIGHT;
      if ((mbe.m_keyboardModifiers == 0) && (isLeft || isRight))
      {
        if(event.type == SDL_MOUSEBUTTONDOWN)
          GetGridPanel().SelectAtomViewPanel(*this); 
        return true;  // Eat all no-mod mouse L and R actions inside us
      }

      */
      return Super::Handle(mbe);
    }
#endif

  };
}

#endif /* ROOTPANEL_H */
