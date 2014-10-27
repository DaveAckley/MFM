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

namespace MFM {

  /**
     Low-level-ish drawing context and methods.  A Drawing is
     something like a GraphicsContext in Java.
   */
  class Drawing
  {
    /**
       Current origin and size for all drawing through this Drawing
     */
    Rect m_rect;

    /**
       Current background color, for clearing and mixing.
     */
    u32 m_bgColor;

    /**
       Current foreground color, for filling and drawing.
     */
    u32 m_fgColor;

    /**
       Target of all drawing operations
     */
    SDL_Surface * m_dest;

    /**
       Current font for text operations
     */
    TTF_Font* m_font;

    static void SetSDLColor(SDL_Color & set, const u32 from) ;
    static void GetSDLColor(const SDL_Color & from, u32 & to) ;

  public:

    static const int WHITE   = 0xffffffff;
    static const int BLACK   = 0xff000000;
    static const int RED     = 0xffff0000;
    static const int GREEN   = 0xff00ff00;
    static const int BLUE    = 0xff0000ff;
    static const int CYAN    = 0xff00ffff;
    static const int MAGENTA = 0xffff00ff;
    static const int YELLOW  = 0xffffff00;

    static const int GREY05  = 0xff0c0c0c;
    static const int GREY10  = 0xff191919;
    static const int GREY20  = 0xff333333;
    static const int GREY30  = 0xff4c4c4c;
    static const int GREY40  = 0xff666666;
    static const int GREY50  = 0xff7f7f7f;
    static const int GREY60  = 0xff999999;
    static const int GREY70  = 0xffb2b2b2;
    static const int GREY80  = 0xffcccccc;
    static const int GREY90  = 0xffe5e5e5;
    static const int GREY95  = 0xfff2f2f2;

    static const int GREY = GREY50;
    static const int DARK_PURPLE = 0xff200020;
    static const int ORANGE = 0xffff6600;

    static u32 HalfColor(u32 input)
    {
      u8 r, g, b;
      r = (input & 0xff0000) >> 17;
      g = (input & 0x00ff00) >> 9;
      b = (input & 0xff) >> 1;

      return 0xff000000 | (r << 16) | (g << 8) | b;
    }

    Drawing(SDL_Surface * dest = 0, TTF_Font * font = 0) ;

    /**
       Return a weighted average of color1 and color2, averaging the
       RGBA components separately.  If percentOfColor1 is 0, return
       color2.  If percentOfColor1 is 100 or greater, return color1.

       Such linear mixing in RGBA works okay when mixing with white or
       black, but in general it isn't necessarily sensible in physical
       or perceptual terms.
     */
    static u32 InterpolateColors(const u32 color1, const u32 color2, u32 percentOfColor1);

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
       Access the current drawing window.
     */
    void GetWindow(Rect & rect) const ;

    /**
       Set the current drawing window.
     */
    void SetWindow(const Rect & rect) ;

    /**
       Reset Drawing based on surface and given font
     */
    void Reset(SDL_Surface * dest, TTF_Font * font);

    /**
       Fill with current background color
     */
    void Clear();

    /**
       Get the current background color.
     */
    u32 GetBackground() const
    {
      return m_bgColor;
    }

    /**
       Set the background color.  Returns prior value
     */
    u32 SetBackground(const u32 color)
    {
      u32 old = m_bgColor;
      m_bgColor = color;
      return old;
    }

    /**
       Get the current foreground color.
     */
    u32 GetForeground() const
    {
      return m_fgColor;
    }

    /**
       Set the foreground color.  Returns prior value
     */
    u32 SetForeground(const u32 color)
    {
      u32 old = m_fgColor;
      m_fgColor = color;
      return old;
    }

    /**
       Get the current font.  May return null.  Text operations will
       fail ILLEGAL_STATE if attempted with a null font.
     */
    TTF_Font* GetFont() const
    {
      return m_font;
    }

    /**
       Set the current font.  Null may be passed in to clear the
       current font.  Text operations will fail ILLEGAL_STATE if
       attempted with a null font.  Returns prior value.
     */
    TTF_Font* SetFont(TTF_Font * newFont)
    {
      TTF_Font* old = m_font;
      m_font = newFont;
      return old;
    }

    /**
     * Set the current font to one loaded by the AssetManager.
     *
     * @param asset The FontAsset of the font to load into this
     *              Drawing instance.
     *
     * @param returns The TTF_Font which was loaded by this Drawing
     *                instance before calling this method.
     */
    TTF_Font* SetFont(FontAsset asset)
    {
      return SetFont(AssetManager::Get(asset));
    }

    /**
       Draw a one-pixel high horizontal line in the current foreground color
     */
    void DrawHLine(int y, int startX, int endX) const;

    /**
       Draw a one-pixel wide vertical line in the current foreground color
     */
    void DrawVLine(int x, int startY, int endY) const;

    /**
       Draw a box with one-pixel lines just inside the given Rect, in
       the current foreground color
     */
    void DrawRectangle(const Rect & rect) const;

    /**
       Fill the given rectangle with the current foreground color
     */
    void FillRect(int x, int y, int w, int h) const;

    /**
       Fill the given rectangle with the given color, without changing
       the foreground color
     */
    void FillRect(int x, int y, int w, int h, u32 color) const;

    /**
       Fill the given rectangle with the current foreground color
     */
    void FillRect(const Rect & rect) const
    {
      FillRect(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
    }

    /**
       Fill a circle of the given radius with the current foreground
       color.  The center of the circle is at (x+w/2, y+h/2).
     */
     void FillCircle(int x, int y, int w, int h, int radius) const;


    /**
       Draw message in the current font using the current foreground
       color.  Fail ILLEGAL_STATE if the current font is null.
     */
    void BlitText(const char* message, UPoint loc, UPoint size) const;

    /**
     * Draw a message in the current font using the current background
     * color as a backing color to another drawn layer using the
     * foreground color. Fail ILLEGAL_STATE if the current font is null.
     */
    void BlitBackedText(const char* message, UPoint loc, UPoint size);

    /**
     * Just like BlitBackedText but center message
     */
    void BlitBackedTextCentered(const char* message, UPoint loc, UPoint size);

    /**
     * Return the SPoint(width, height) of message when rendered in
     * the current font, or SPoint(-1,-1) if any problem occurs
     */
    SPoint GetTextSize(const char* message) ;

    /**
     * Return the SPoint(width, height) of message when rendered in
     * thisFont, or SPoint(-1,-1) if any problem occurs
     */
    SPoint GetTextSizeInFont(const char* message, TTF_Font * thisFont) ;

    /**
     * Draw a specified image to a specified part of the screen.
     */
    void BlitImage(SDL_Surface* image, UPoint loc, UPoint maxSize) const;

    /**
     * Draw a specified Asset (corresponding to an SDL_Surface*) to the screen.
     */
    void BlitAsset(Asset asset, UPoint loc, UPoint maxSize) const;


    static void Convert(const Rect & rect, SDL_Rect & toFill) ;

  private:
    // Let's try to deprecate SetPixel.  Bounds-checking every pixel is
    // slow, and SDL_FillRect is (like SSE) fast and also clips against
    // the surface cliprect so it's bounds-checking anyway?
    static inline void SetPixel(SDL_Surface* dest,
                                u32 x, u32 y,
                                u32 color)
    {
      if(x >= 0 && y >= 0 && x < (u32)dest->w && y < (u32)dest->h)
        {
          ((u32*)dest->pixels)[x + y * dest->w] = color;
        }
    }
  };
} /* namespace MFM */

#endif /*DRAWING_H*/
