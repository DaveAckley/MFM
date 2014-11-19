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
#include "SDL.h"
#include "SDL_ttf.h"
#include "Point.h"
#include "Panel.h"
#include "Drawing.h"

namespace MFM
{
  /**
   * A Panel which behaves as a button, built with abstract behavior
   * upon clicks.
   */
  class AbstractButton : public Panel
  {
   private:

    /**
     * Default color for the border of every AbstractButton
     */
    static const u32 BUTTON_BORDER_COLOR = Drawing::GREY10;

    /**
     * Default color for the background of every AbstractButton
     */
    static const u32 BUTTON_BACKGROUND_COLOR = Drawing::GREY20;

    /**
     * Default color for the foreground of every AbstractButton
     */
    static const u32 BUTTON_COLOR = Drawing::GREY90;

    /**
     * The size of this AbstractButton when being rendered and
     * processing mouse hits.
     */
    SPoint m_dimensions;

    /**
     * The location of this AbstractButton relative to the Panel that
     * is containing it.
     */
    SPoint m_location;

    /**
     * The text that will render on this AbstractButton .
     */
    const char* m_text;

    /**
     * An optional icon. If not NULL, will render instead of the default
     * rendering scheme.
     */
    SDL_Surface* m_icon;

    /**
     * A flag indicating whether or not this AbstractButton is enabled
     * and therefore able to be clicked.
     */
    bool m_enabled;

    bool m_justClicked;

    /**
     * Initializes this AbstractButton with default values.
     */
    void Init() ;

  public:

    /**
     * Constructs an AbstractButton with default parameters and
     * default coloring scheme.
     */
    AbstractButton();

    /**
     * Constructs an AbstractButton with given text to be rendered
     * upon it and default coloring scheme.
     *
     * @param text The text to render on top of this AbstractButton
     *             upon rendering.
     */
    AbstractButton(const char* text);

    /**
     * Constructs an AbstractButton with a given icon to be rendered
     * instead of the default rendering scheme.
     *
     * @param icon A pointer to the SDL_Surface which will be rendered
     *             upon rendering of this AbstactButton.
     */
    AbstractButton(SDL_Surface* icon);

    /**
     * Constructs an AbstractButton with a given string and a given
     * icon. Upon rendering, The icon will be rendered, then the
     * string will be rendered on top of it.
     *
     * @param text The string to render on top of this AbstractButton
     *             upon rendering.
     *
     * @param icon The icon which will render as this AbstractButton
     *             upon rendering.
     */
    AbstractButton(const char* text, SDL_Surface* icon);

    /**
     * Deconstructs this AbstractButton.
     */
    ~AbstractButton();

    /**
     * Sets the location of this AbstractButton, relative to the Panel
     * which it is located inside of, at which to render this
     * AbstractButton and process mouse hits.
     *
     * @param location The new location that this AbstractButton will
     *                 reside at.
     */
    void SetLocation(const SPoint& location)
    {
      m_location = location;
    }

    /**
     * Sets the icon of this AbstractButton which will render in its
     * place upon rendering.
     *
     * @param icon A pointer to the SDL_Surface which will render upon
     *             rendering of this AbstractButton.
     */
    void SetIcon(SDL_Surface* icon)
    {
      m_icon = icon;
    }

    /**
     * Gets the icon that is currently being rendered by this
     * AbstractButton .
     *
     * @returns The icon that is currently being rendered by this
     *          AbstractButton .
     */
    SDL_Surface* GetIcon()
    {
      return m_icon;
    }

    /**
     * Sets the size of this AbstractButton which will affect how it
     * is rendered on the screen and how mouse hits are processed.
     *
     * @param dimensions The new size of this AbstractButton .
     */
    void SetDimensions(const SPoint& dimensions)
    {
      m_dimensions = dimensions;
      Panel::SetDimensions(dimensions.GetX(), dimensions.GetY());
    }

    /**
     * Sets whether or not this AbstractButton will be enabled, and
     * therefore whether or not it will respond to mouse hits.
     *
     * @param isEnabled If \c true , will allow this AbstractButton to
     *                  respond to mouse hits. If \c false , will
     *                  disable this feature.
     */
    void SetEnabled(bool isEnabled)
    {
      m_enabled = isEnabled;
    }

    /**
     * Sets the text that will be rendered on top of this
     * AbstractButton upon rendering.
     *
     * @param text The new text that will be rendered on top of this
     * AbstractButton upon rendering.
     */
    void SetText(const char* text)
    {
      m_text = text;
    }

    /**
     * Gets the text that is currently being rendered on top of this
     * AbstractButton upon rendering.
     *
     * @returns The text that is currently being rendered on top of this
     * AbstractButton upon rendering.
     */
    const char* GetText()
    {
      return m_text;
    }

    /**
     * Checks to see whether or not this AbstractButton is enabled and
     * is therefore responding to mouse hits.
     *
     * @returns \c true if this AbstractButton is enabled and is
     *          therefore responding to mouse hits, else \c false .
     */
    bool IsEnabled() const
    {
      return m_enabled;
    }

    /**
     * Checks to see whether or not a given SPoint lies within the
     * bounds of this AbstractButton. This is used during mouse hit
     * processing to see whether or not this AbstractButton has been
     * clicked on.
     *
     * @param pt The SPoint in question of existing within the bounds
     *           of this AbstractButton.
     */
    bool Contains(SPoint& pt);

    /////////
    //// Panel Methods

    virtual bool Handle(MouseButtonEvent & event)
    {
      if (IsEnabled())
      {
        if(event.m_event.button.button == SDL_BUTTON_WHEELUP ||
           event.m_event.button.button == SDL_BUTTON_WHEELDOWN)
        {
          return false; /* Don't take wheel events. */
        }
        if (event.m_event.type == SDL_MOUSEBUTTONUP)      // Execute on up
        {
          m_justClicked = true;
          OnClick(event.m_event.button.button);
          return true;                                     // We took it
        }
        if (event.m_event.type == SDL_MOUSEBUTTONDOWN)     // But eat down too
        {
          OnPress(event.m_event.button.button);
          return true;
        }
      }
      return false;
    }

    virtual void HandleResize(const UPoint& parentSize)
    {
      /* As a button, do nothing. */
    }

    bool PaintClickHighlight(Drawing& d)
    {
      if(m_justClicked)
      {
        m_justClicked = false;
        d.SetForeground(Drawing::YELLOW);
        d.FillRect(0, 0,
                   this->Panel::GetDimensions().GetX(), this->Panel::GetDimensions().GetY());
        return true;
      }
      return false;
    }

    virtual void PaintComponentNonClick(Drawing& d);

    virtual void PaintComponent(Drawing & d);

    /**
     * Pure abstract behavior method. This is called if this
     * AbstractButton is enabled and has decided that it has been
     * clicked on . Overriding this method allows the subclass of this
     * AbstractButton to behave in any way it pleases upon being clicked.
     *
     * @param button The SDL mouse button that has been pressed to
     *               generate this mouse hit, i.e. SDL_BUTTON_LEFT ,
     *               SDL_BUTTON_MIDDLE, or SDL_BUTTON_RIGHT .
     */
    virtual void OnClick(u8 button) = 0;

    /**
     * This is called when this AbstractButton is enabled and has
     * decided the button is pressed on it . Overriding this method
     * allows the subclass of this AbstractButton to behave in any way
     * it pleases upon button down.
     *
     * @param button The SDL mouse button that has been pressed to
     *               generate this mouse hit, i.e. SDL_BUTTON_LEFT ,
     *               SDL_BUTTON_MIDDLE, or SDL_BUTTON_RIGHT .
     */
    virtual void OnPress(u8 button)
    { }
  };
}

#endif /* ABSTRACTBUTTON_H */
