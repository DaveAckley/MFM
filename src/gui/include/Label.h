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
#include "MovablePanel.h"
#include "Drawing.h"
#include "Fail.h"

namespace MFM
{
  /**
   * A Panel which displays some text and possibly an icon.
   */
  class Label : public MovablePanel
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
     * An optional icon. If not IMAGE_ASSET_NONE, will render along
     * with the text (if any).
     */
    ImageAsset m_iconAsset;

    /**
     * A zoomable icon asset. If its image is not IMAGE_ASSET_NONE,
     * will render instead of the m_iconAsset with the text (if any).
     */
    IconAsset m_zIconAsset;

    /**
     * Where to render the icon, if present.  Default (0,0)
     */
    SPoint m_iconPosition;

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
     * @param icon An ImageAsset to use when rendering this
     *             AbstactButton.
     */
    Label(ImageAsset icon);

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
    Label(const char* text, ImageAsset icon);

    /**
     * Destructor.
     */
    ~Label();


    /**
     * Get access to the zoomable IconAsset of this label, to configure it.  
     */
    IconAsset & GetIconAsset() 
    {
      return m_zIconAsset;
    }

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
     * Sets the icon of this Label which will render in its place upon
     * rendering, specified by a loaded Asset.
     *
     * @param icon The ImageAsset to use as this Label's image, or
     *             IMAGE_ASSET_NONE to disable icon rendering.
     */
    void SetIconAssetDEPRECATED(ImageAsset a)
    {
      m_iconAsset = a;
    }

    /**
     * Sets the icon of this Label which will render in its place upon
     * rendering, specified by a loaded Asset.
     *
     * @param icon The ImageAsset to use as this Label's image, or
     *             IMAGE_ASSET_NONE to disable icon rendering.
     */
    void SetIconSlot(MasterIconZSheetSlot a)
    {
      m_zIconAsset.SetIconSlot(a);
    }

    /**
     * Gets the icon that is currently being rendered by this
     * Label .
     *
     * @returns The icon that is currently being rendered by this
     *          Label .
     */
    ImageAsset GetIconAsset() const
    {
      return m_iconAsset;
    }

    /**
     * Gets the current position of the icon currently being rendered
     * by this Label , if any
     *
     * @returns The position the icon will be rendered at if it exists
     */
    SPoint GetIconPosition() const
    {
      return m_iconPosition;
    }

    /**
     * Sets the position the icon of this Label will be rendered at,
     * if it is being used.
     *
     * @param pt The point within the Label as which to render the
     * icon
     */
    void SetIconPosition(const SPoint pt)
    {
      m_iconPosition = pt;
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
