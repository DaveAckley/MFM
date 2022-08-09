/*                                              -*- mode:C++ -*-
  Drawable.h Abstract drawing routines
  Copyright (C) 2014, 2020 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2020 The Living Computation Foundation.

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
  \file Drawable.h Abstract drawing routines
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "itype.h"
#include "Rect.h"

namespace MFM {

  enum DrawSiteType { //Migrated from ../gui for T2 accessibility
    DRAW_SITE_ELEMENT,         //< Static color of event layer atom
    DRAW_SITE_ATOM_1,          //< Dynamic per-atom rendering type 1
    DRAW_SITE_ATOM_2,          //< Dynamic per-atom rendering type 2
    DRAW_SITE_BASE,            //< Static color of base atom
    DRAW_SITE_BASE_1,          //< Dynamic base-atom rendering type 1
    DRAW_SITE_BASE_2,          //< Dynamic base-atom rendering type 2
    DRAW_SITE_LIGHT_TILE,      //< Light grey rendering of tile regions
    DRAW_SITE_DARK_TILE,       //< Dark grey rendering of hidden regions
    DRAW_SITE_CHANGE_AGE,      //< CubeHelix rendering of events-since-change
    DRAW_SITE_PAINT,           //< Last color painted on site
    DRAW_SITE_NONE,            //< Do not draw atoms at all
    DRAW_SITE_BLACK,           //< Fill with black
    DRAW_SITE_WHITE,           //< Fill with white
    DRAW_SITE_TYPE_COUNT
  };

  enum DrawSiteShape { //Migrated from ../gui for T2 accessibility
    DRAW_SHAPE_FILL,           //< Flood fill site entirely (square)
    DRAW_SHAPE_CIRCLE,         //< Draw circle touching site edges
    DRAW_SHAPE_CDOT,           //< Draw small centered dot
    DRAW_SHAPE_COUNT
  };

  /**
     Low-level-ish Drawable context and methods.  A Drawable is
     something like a GraphicsContext in Java.
   */
  class Drawable
  {
  protected:
    /**
       Current origin and size for all Drawable through this Drawable
     */
    Rect m_rect;

    /**
       Current background color, for clearing and mixing.
     */
    u32 m_bgColor;

    /**
       Current foreground color, for filling and Drawable.
     */
    u32 m_fgColor;

    /**
       Scale factor to apply to (currently only font) Drawable operations.
     */
    u32 m_drawScaleDits;

    /**
       Current dits per site scale factor
    */
    u32 m_ditsPerSite;

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
    static const int LIGHTER_DARK_PURPLE = 0xff280028;
    static const int ORANGE = 0xffff6600;

    static u32 HalfColor(u32 input)
    {
      u8 r, g, b;
      r = (input & 0xff0000) >> 17;
      g = (input & 0x00ff00) >> 9;
      b = (input & 0xff) >> 1;

      return 0xff000000 | (r << 16) | (g << 8) | b;
    }

    Drawable() ;
    virtual ~Drawable() { }

    void Reset() ;

    u32 GetDitsPerSite() const { return m_ditsPerSite; }

    void SetDitsPerSite(u32 ditsPerSite) { m_ditsPerSite = ditsPerSite; }

    /**
       Access the current Drawable window.
     */
    void GetWindow(Rect & rect) const ;

    /**
       Set the current Drawable window.
     */
    void SetWindow(const Rect & rect) ;

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
       Draw a one-pixel high horizontal line in the current foreground color
     */
    void DrawHLine(int y, int startX, int endX) const
    {
      DrawHLine(y,startX,endX,m_fgColor);
    }

    /**
       Draw a one-pixel high horizontal line in the given color
     */
    void DrawHLine(int y, int startX, int endX, u32 color) const;


    void DrawHLineDit(int y, int startX, int endX) const
    {
      DrawHLineDit(y, startX, endX, m_fgColor);
    }

    void DrawHLineDit(int y, int startX, int endX, u32 color) const
    {
      DrawHLine(MapDitToPix(y), MapDitToPix(startX), MapDitToPix(endX), color);
    }

    /**
       Draw a one-pixel wide vertical line in the current foreground color
     */
    void DrawVLine(int x, int startY, int endY) const
    {
      DrawVLine(x,startY,endY,m_fgColor);
    }

    /**
       Draw a one-pixel wide vertical line in the given color
     */
    void DrawVLine(int x, int startY, int endY, u32 color) const ;


    void DrawVLineDit(int x, int startY, int endY) const
    {
      DrawVLineDit(x, startY, endY, m_fgColor);
    }

    void DrawVLineDit(int x, int startY, int endY, u32 color) const
    {
      DrawVLine(MapDitToPix(x), MapDitToPix(startY), MapDitToPix(endY), color);
    }

    /**
       Draw a solid one pixel line in the current foreground color
     */
    void DrawLineDit(int startX, int startY, int endX, int endY) const
    {
      DrawLineDitColor(startX, startY, endX, endY, m_fgColor);
    }

    /**
       Draw a solid one pixel line in the given color
     */
    void DrawLineDitColor(int startX, int startY, int endX, int endY, u32 color) const
    {
      DrawMaskedLineDitColor(startX, startY, endX, endY, color, -1);
    }

    /**
       Draw a solid or dashed one pixel line in the current foreground color
     */
    void DrawMaskedLineDit(int startX, int startY, int endX, int endY, u32 mask) const
    {
      DrawMaskedLineDitColor(startX, startY, endX, endY, m_fgColor, mask);
    }

    /**
       Draw a solid or dashed one pixel line in the given color
     */
    void DrawMaskedLineDitColor(int startX, int startY, int endX, int endY, u32 color, u32 mask) const
    {
      DrawScaledMaskedLineDitColor(startX, startY, endX, endY, color, color, mask, 1u, 1u);
    }

    /**
       Draw a solid or dashed width pixel line in the given color
     */
    void DrawMaskedLineDitColor(int startX, int startY, int endX, int endY, u32 color, u32 mask, u32 maskWidth, u32 width) const
    {
      DrawScaledMaskedLineDitColor(startX, startY, endX, endY, color, color, mask, maskWidth, width);
    }

    /**
       Draw a solid or one pixel dashed line based on mask scaled by
       maskDit, using the given 'onColor', plus an 'offColor' that is
       treated as 'transparent' if it's the same as 'onColor'.
       Examples:

         - Solid black line:   on==0xff000000, off==ANYTHING,   mask==0xffffffff
         - Solid red line:     on==0xffff0000, off==ANYTHING,   mask==0xffffffff
         - Long green dashes:  on==0xff00ff00, off==0xff00ff00, mask==0xfffff000
         - Short red dashes:   on==0xffff0000, off==0xffff0000, mask==0xf0f0f0f0
         - Black/blue dashes:  on==0xff000000, off==0xff0000ff, mask==0xffff0000
         - Tiny B/W dashes:    on==0xff000000, off==0xffffffff, mask==0x33333333
         - Yellow/black pixels:on==0xffffff00, off==0xff000000, mask==0x55555555

     */
    void DrawScaledMaskedLineDitColor(int startX, int startY, int endX, int endY, u32 onColor, u32 offColor, u32 mask, u32 maskDits, u32 widthDits) const;


    /**
       Draw a box with one-pixel lines just inside the given Rect, in
       the current foreground color
     */
    void DrawRectangle(const Rect & rect) const;

    /**
       Draw a box with one-pixel lines just inside the given Rect,
       measured in dits, in the current foreground color
     */
    void DrawRectDit(const Rect & rect) const;

    /**
       Fill the given rectangle with the current foreground color
     */
    void FillRect(int x, int y, int w, int h) const;

    /**
       Fill the given rectangle with the given color, without changing
       the foreground color
     */
    virtual void FillRect(int x, int y, int w, int h, u32 color) const = 0;

    /**
       Fill the given rectangle, measured in dit, with the given
       color, without changing the foreground color
     */
    void FillRectDit(int x, int y, int w, int h, u32 color) const;

    /**
       Fill the given rectangle, measured in dit, with the given
       color, without changing the foreground color.
     */
    void FillRectDit(const Rect & rectDit, u32 color) const ;

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
       Fill a circle of the given radius with the current foreground
       color.  The center of the circle is at (x+w/2, y+h/2).
     */
    void FillCircleDit(const Rect & rect, u32 radiusDits, u32 color) const;

    /*
      Support for cheapo (i.e., no antialiasing) subpixel rendering
     */
    static const u32 DIT_PER_PIX = 256;

    static s32 MapPixToDit(s32 pix) { return pix * (s32) DIT_PER_PIX; }

    static u32 MapPixToDit(u32 pix) { return pix * DIT_PER_PIX; }

    static s32 MapDitToPix(s32 dit) { return (s32) (dit + DIT_PER_PIX / 2) / (s32) DIT_PER_PIX; }
    static u32 MapDitToPix(u32 dit) { return (dit + DIT_PER_PIX / 2) / DIT_PER_PIX; }

    static s32 MapDitToPixCeiling(s32 dit) { return (s32) (dit + DIT_PER_PIX - 1) / (s32) DIT_PER_PIX; }
    static u32 MapDitToPixCeiling(u32 dit) { return (dit + DIT_PER_PIX - 1) / DIT_PER_PIX; }

    static s32 MapDitToPixFloor(s32 dit) { return (s32) (dit + 0) / (s32) DIT_PER_PIX; }
    static u32 MapDitToPixFloor(u32 dit) { return (dit + 0) / DIT_PER_PIX; }

    static SPoint MapPixToDit(const SPoint pix)
    {
      return SPoint(MapPixToDit(pix.GetX()), MapPixToDit(pix.GetY()));
    }

    static SPoint MapDitToPix(const SPoint & dit)
    {
      return SPoint(MapDitToPix(dit.GetX()), MapDitToPix(dit.GetY()));
    }

    static UPoint MapDitToPix(const UPoint & pdit) {
      return UPoint(MapDitToPix(pdit.GetX()),MapDitToPix(pdit.GetY()));
    }

    static UPoint MapPixToDit(const UPoint & pix) {
      return UPoint(MapPixToDit(pix.GetX()),MapPixToDit(pix.GetY()));
    }

    static Rect MapDitToPix(const Rect & rectdit) {
      return Rect(MapDitToPix(rectdit.GetPosition()), MapDitToPix(rectdit.GetSize()));
    }

  private:

  };
} /* namespace MFM */

#endif /*DRAWABLE_H*/
