/*                                              -*- mode:C++ -*-
  AbstractButton.h Clickable Label
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
  \file AbstractButton.h Clickable Label
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "itype.h"
#include "Label.h"

namespace MFM
{
  /**
   * A Label which behaves as a button, built with abstract behavior
   * upon clicks.
   */
  class AbstractButton : public Label
  {
   private:
    typedef Label Super;

    /**
     * A flag indicating whether or not this AbstractButton is enabled
     * and therefore able to be clicked.
     */
    bool m_enabled;

    u32 m_enabledFg;
    u32 m_enabledBg;

    u32 m_disabledFg;
    u32 m_disabledBg;

    bool m_justClicked;

  public:

    void Init() ;

    /**
     * Constructs an AbstractButton with default parameters and
     * default coloring scheme.
     */
    AbstractButton() ;

    /**
     * Constructs an AbstractButton with given text to be rendered
     * upon it and default coloring scheme.
     *
     * @param text The text to render on top of this AbstractButton
     *             upon rendering.
     */
    AbstractButton(const char* text) ;

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
     * Destructs this AbstractButton.
     */
    ~AbstractButton();

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

    void SetDisabledForeground(u32 rgb) { m_disabledFg = rgb; }
    void SetDisabledBackground(u32 rgb) { m_disabledBg = rgb; }
    void SetEnabledForeground(u32 rgb) { m_enabledFg = rgb; }
    void SetEnabledBackground(u32 rgb) { m_enabledBg = rgb; }

    u32 GetDisabledForeground() const { return m_disabledFg; }
    u32 GetDisabledBackground() const { return m_disabledBg; }
    u32 GetEnabledForeground() const { return m_enabledFg; }
    u32 GetEnabledBackground() const { return m_enabledBg; }

    u32 SetForeground(const u32 color)
    {
      u32 old = Super::SetForeground(color);
      SetEnabledForeground(color);
      return old;
    }

    u32 SetBackground(const u32 color)
    {
      u32 old = Super::SetBackground(color);
      SetEnabledBackground(color);
      return old;
    }

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
