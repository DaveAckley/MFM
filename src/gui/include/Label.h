/*                                              -*- mode:C++ -*-
  Label.h Text-displaying panel
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
  \file Label.h Floating info panel
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef LABEL_H
#define LABEL_H

#include "itype.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "Point.h"
#include "Panel.h"
#include "Drawing.h"
#include "Fail.h"

namespace MFM
{
  /**
   * A Panel which does nothing except display some text.
   */
  class Label : public Panel
  {
   private:

    /**
     * Default color for the border of every Label
     */
    static const u32 LABEL_BORDER_COLOR = Drawing::GREY10;

    /**
     * Default color for the background of every Label
     */
    static const u32 LABEL_BACKGROUND_COLOR = Drawing::GREY20;

    /**
     * Default color for the foreground of every Label
     */
    static const u32 LABEL_COLOR = Drawing::GREY90;

    /**
     * The text that will render on this Label .
     */
    OString64 m_text;

    /**
     * An optional icon. If not NULL, will render instead of the default
     * rendering scheme.
     */
    SDL_Surface* m_icon;

    /**
     * Initializes this Label
     */
    void Init() ;

  public:

    /**
     * Constructs a Label with default parameters and
     * default coloring scheme.
     */
    Label();

    /**
     * Constructs a Label with given text to be rendered
     * upon it and default coloring scheme.
     *
     * @param text The text to render on top of this Label
     *             upon rendering.
     */
    Label(const char* text);

    /**
     * Constructs a Label with a given icon to be rendered
     * instead of the default rendering scheme.
     *
     * @param icon A pointer to the SDL_Surface which will be rendered
     *             upon rendering of this AbstactButton.
     */
    Label(SDL_Surface* icon);

    /**
     * Constructs a Label with a given string and a given
     * icon. Upon rendering, The icon will be rendered, then the
     * string will be rendered on top of it.
     *
     * @param text The string to render on top of this Label
     *             upon rendering.
     *
     * @param icon The icon which will render as this Label
     *             upon rendering.
     */
    Label(const char* text, SDL_Surface* icon);

    /**
     * Destructor.
     */
    ~Label();


    /**
     * Sets the location of this Label, relative to the Panel
     * which it is located inside of, at which to render this
     * Label and process mouse hits.
     *
     * @param location The new location that this Label will
     *                 reside at.
     */
    void SetLocation(const SPoint& location)
    {
      SetRenderPoint(location);
    }

    /**
     * Sets the icon of this Label which will render in its
     * place upon rendering.
     *
     * @param icon A pointer to the SDL_Surface which will render upon
     *             rendering of this Label.
     */
    void SetIcon(SDL_Surface* icon)
    {
      m_icon = icon;
    }

    /**
     * Sets the icon of this Label which will render in its
     * place upon rendering, specified by a loaded Asset.
     *
     * @param icon The Asset representing the SDL_Surface to use as
     *             this Label's image.
     */
    void SetIcon(Asset a)
    {
      SetIcon(AssetManager::Get(a));
    }

    /**
     * Gets the icon that is currently being rendered by this
     * Label .
     *
     * @returns The icon that is currently being rendered by this
     *          Label .
     */
    SDL_Surface* GetIcon()
    {
      return m_icon;
    }

    /**
     * Sets the size of this Label which will affect how it
     * is rendered on the screen and how mouse hits are processed.
     *
     * @param dimensions The new size of this Label .
     */
    void SetDimensions(const SPoint& dimensions)
    {
      Panel::SetDimensions(dimensions.GetX(), dimensions.GetY());
    }

    /**
     * Sets the text that will be rendered on top of this
     * Label upon rendering.
     *
     * @param text The new text that will be rendered on top of this
     * Label upon rendering.
     */
    void SetText(const char* text)
    {
      MFM_API_ASSERT_NONNULL(text);
      m_text.Reset();
      m_text.Print(text);
    }

    /**
     * Gets the text that is currently being rendered on top of this
     * Label upon rendering.
     *
     * @returns The text that is currently being rendered on top of this
     * Label upon rendering.
     */
    const char* GetText()
    {
      return m_text.GetZString();
    }

    /**
     * Checks to see whether or not a given SPoint lies within the
     * bounds of this Label.
     *
     * @param pt The SPoint in question of existing within the bounds
     *           of this Label.
     */
    bool Contains(SPoint& pt);

    virtual void HandleResize(const UPoint& parentSize)
    {
      /* Labels don't resize automatically? */
    }

    virtual void PaintComponent(Drawing & d);

  };
}

#endif /* LABEL_H */
