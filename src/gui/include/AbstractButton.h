/*                                              -*- mode:C++ -*-
  AbstractButton.h Clickable Panel
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
  \file AbstractButton.h Clickable Panel
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "itype.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "Point.h"
#include "Panel.h"
#include "Drawing.h"

namespace MFM
{
  class AbstractButton : public Panel
  {
  private:

    static const u32 BUTTON_BORDER_COLOR = Drawing::GREY10;
    static const u32 BUTTON_BACKGROUND_COLOR = Drawing::GREY20;
    static const u32 BUTTON_COLOR = Drawing::GREY90;

    void Init() ;

    SPoint m_dimensions;

    SPoint GetTextSize(TTF_Font * font, const char * text);

    SPoint m_location;

    const char* m_text;

    SDL_Surface* m_icon;

    bool m_enabled;

  public:

    AbstractButton();

    AbstractButton(const char* text);

    AbstractButton(SDL_Surface* icon);

    AbstractButton(const char* text, SDL_Surface* icon);

    void SetLocation(const SPoint& location)
    {
      m_location = location;
    }

    void SetIcon(SDL_Surface* icon)
    {
      m_icon = icon;
    }

    void SetDimensions(const SPoint& dimensions)
    {
      m_dimensions = dimensions;
    }

    void SetEnabled(bool isEnabled)
    {
      m_enabled = isEnabled;
    }

    void SetText(const char* text)
    {
      m_text = text;
    }

    bool IsEnabled() const
    {
      return m_enabled;
    }

    /////////
    //// Panel Methods

    virtual bool Handle(MouseButtonEvent & event)
    {
      if (IsEnabled() && event.m_event.type == SDL_MOUSEBUTTONUP) {
        OnClick(event.m_event.button.button);
        return true;
      }
      return false;
    }

    virtual void HandleResize(const UPoint& parentSize)
    {
      /* As a button, do nothing. */
    }

    virtual void PaintComponent(Drawing & config) ;

    virtual void OnClick(u8 button) = 0;

    ~AbstractButton();

    bool Contains(SPoint& pt);
  };
}

#endif /* ABSTRACTBUTTON_H */
