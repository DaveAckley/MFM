/*                                              -*- mode:C++ -*-
  Drawing.h SDL_Surface rendering toolkit
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
  \file Drawing.h SDL_Surface rendering toolkit
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef DRAWING_H
#define DRAWING_H

#include "AssetManager.h"
#include "itype.h"
#include "SDL.h"         /* For SDL_Surface, SDL_Color */
#include "SDL_ttf.h"     /* For TTF_Font, at least */
#include "Rect.h"
#include "Tile.h"
#include "Drawable.h"

namespace MFM {

  /**
     SDL-specific extensions to Drawable, mostly involving fonts and
     images, which are not modeled by Drawable.
   */
  class Drawing : public Drawable
  {
    /**
       Target of all drawing operations
     */
    SDL_Surface * m_dest;

    /**
       Current font asset for text operations
     */
    FontAsset m_fontAsset;

    /**
       Scale factor to apply to (currently only font) drawing operations.
     */
    u32 m_drawScaleDits;
    
    static void SetSDLColor(SDL_Color & set, const u32 from) ;
    static void GetSDLColor(const SDL_Color & from, u32 & to) ;

  public:

    Drawing(SDL_Surface * dest = 0, FontAsset font = FONT_ASSET_NONE) ;

    /**
       Update the drawing window.  The GetX() and GetY() of rect
       provide the position of (0,0), relative to the previous window,
       and the GetWidth() and GetHeight() of rect provide the bounds
       for drawing (clipped against the prior bounds, if any).
     */
    void TransformWindow(const Rect & rect) ;

    /**
       Compute a the drawing window update.  This is what
       TransformWindow uses to control drawing, and also what the
       mouse dispatching uses to determine who sees events.
     */
    static void TransformWindow(const Rect & existing, const Rect & newTransform, Rect & result) ;

    /**
       Set the scale factor (currently affects only fonts) of the
       drawing window.  dits == MapPixToDit(1u) is no scaling, larger
       to scale up, smaller to scale down.  Returns old scale factor
     */
    u32 SetZoomDits(u32 dits) ;

    /**
       Get the scale factor (currently affects only fonts) of the drawing window. 
       \sa SetZoomDits
     */
    u32 GetZoomDits() const ;

    /**
       Reset Drawing based on surface and given font
     */
    void Reset(SDL_Surface * dest, FontAsset font);

    /**
     * Set the current font to one loaded by the AssetManager.
     *
     * @param asset The FontAsset of the font to load into this
     *              Drawing instance.
     *
     * @param returns The TTF_Font which was loaded by this Drawing
     *                instance before calling this method.
     */
    FontAsset SetFont(FontAsset asset)
    {
      FontAsset old = m_fontAsset;
      m_fontAsset = asset;
      return old;
    }

    FontAsset GetFont() const
    {
      return m_fontAsset;
    }

    /**
       Fill the given rectangle with the given color, without changing
       the foreground color
     */
    virtual void FillRect(int x, int y, int w, int h, u32 color) const;

    /* And pick up other FillRects from the base class geez */
    using Drawable::FillRect;

    /**
     * Draw message in the current font (or the given \c infont, if
     * non-null) using the current foreground color, clipping to the
     * given size, starting at the given loc.  Fail ILLEGAL_STATE if
     * the current font is null.
     */
    void BlitText(const char* message, SPoint loc, UPoint size, TTF_Font * infont = 0) const;

    /**
     * Draw a message in the current font (or the given \c infont, if
     * non-null) using the current background color as a backing color
     * to another drawn layer using the foreground color, clipping to
     * the given size, starting at the * given loc. Fail ILLEGAL_STATE
     * if the current font is null.
     */
    void BlitBackedText(const char* message, SPoint loc, UPoint size, TTF_Font * infont = 0);

    /**
     * Just like BlitBackedText but center message on loc + size / 2
     */
    void BlitBackedTextCentered(const char* message, SPoint loc, UPoint size, TTF_Font * infont = 0);

    /**
     * Return the SPoint(width, height) of message when rendered in
     * the current font (or the given \c infont, if non-null), or
     * SPoint(-1,-1) if any problem occurs
     */
    SPoint GetTextSize(const char* message, TTF_Font * infont = 0) ;

    /**
     * Return the SPoint(width, height) of message when rendered in
     * thisFont, or SPoint(-1,-1) if any problem occurs
     */
    SPoint GetTextSizeInFont(const char* message, FontAsset font) ;

    /**
     * Return the SPoint(width, height) of message when rendered in
     * thisFont, or SPoint(-1,-1) if any problem occurs
     */
    SPoint GetTextSizeInTTFFont(const char* message, TTF_Font & font) ;

    /**
     * Draw a specified image to a specified part of the screen.
     */
    void BlitImage(SDL_Surface* image, SPoint loc, UPoint maxSize) const;

    /**
     * Tile copies of a specified image to a specified part of the screen.
     */
    void BlitImageTiled(SDL_Surface* image, const Rect & destRegion) const;

    /**
     * Draw a specified ImageAsset (corresponding to an SDL_Surface*) to the screen.
     */
    void BlitImageAsset(ImageAsset asset, SPoint loc, UPoint maxSize) const;

    /**
     * Draw a specified ImageAsset at loc in its own maximum size.
     */
    void BlitImageAsset(ImageAsset asset, SPoint loc) const;

    /**
     * Tile copies of a specified image to a specified part of the screen.
     */
    void BlitImageAssetTiled(ImageAsset asset, const Rect & destRegion) const;

    /** 
      Draw a portion of a source image to a destination location on
      the screen.  The \c srcRegion determines the size and position
      within the \c src image that may be copied.  The \c destLoc
      determines where the copied image portion is to be placed on the
      screen.  The rectangle implied by the srcRegion size and the
      destLoc position is clipped against the current drawing window
      and the remaining area, if any, is drawn.
     
     */
    void BlitSubImage(SDL_Surface* src, const Rect & srcRegion, SPoint destLoc) const;

    /** 
      Draw \c icon at (about) \c size high at \c destLoc on the
      screen.  If icon.IsEnabled() is true, draw the original 'bright'
      version of the icon, otherwise draw the 'dimmed' version in
      which all colors are compressed towards 50% grey.
     */
    void BlitIconAsset(const IconAsset & icon, u32 size, SPoint destLoc) const;

    static void Convert(const Rect & rect, SDL_Rect & toFill) ;

  private:

    // Let's try to deprecate SetPixel.  Bounds-checking every pixel is
    // slow, and SDL_FillRect is (like SSE) fast and also clips against
    // the surface cliprect so it's bounds-checking anyway?
#if 0
    static inline void SetPixel(SDL_Surface* dest,
                                u32 x, u32 y,
                                u32 color)
    {
      if(x < (u32)dest->w && y < (u32)dest->h)
      {
        ((u32*)dest->pixels)[x + y * dest->w] = color;
      }
    }
#endif
  };
} /* namespace MFM */

#endif /*DRAWING_H*/
