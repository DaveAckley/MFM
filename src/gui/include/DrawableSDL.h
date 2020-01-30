/*                                              -*- mode:C++ -*-
  DrawableSDL.h Wrapper to expose SDL rect filling
  Copyright (C) 2020 The Regents of the University of New Mexico.  All rights reserved.

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
  \file DrawableSDL.h Wrapper to expose SDL rect filling
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef DRAWABLESDL_H
#define DRAWABLESDL_H

#include "Drawable.h"
#include "Drawing.h"
#include "EventWindowRenderer.h"
#include "UlamContextRestricted.h"

namespace MFM {

  /**
     Concrete Drawable that draws via SDL
   */
  class DrawableSDL : public Drawable
  {
    
    Drawing & m_drawing;
    SPoint m_ditOrigin;

  public:

    DrawableSDL(Drawing & drawing) : m_drawing(drawing) { }

    void SetDitOrigin(const SPoint ditOrigin) { m_ditOrigin = ditOrigin; }
    SPoint GetDitOrigin() const { return m_ditOrigin; }

    /**
       Fill the given pixel rectangle with the given color, without changing
       the foreground color
     */
    void FillRect(int x, int y, int w, int h, u32 color) const
    {
      m_drawing.FillRect(MapDitToPix(m_ditOrigin.GetX())+x,
                         MapDitToPix(m_ditOrigin.GetY())+y,
                         w,h,color);
    }

  };

  template <class EC>
  class EventWindowRendererSDL : public EventWindowRenderer<EC> {
    DrawableSDL & m_drawable;
  public:
    EventWindowRendererSDL(DrawableSDL & dsl) : m_drawable(dsl) { }
    virtual Drawable * getDrawableOrNull() const { return &m_drawable; }
  };

  template <class EC>
  class UlamContextRestrictedSDL : public UlamContextRestricted<EC> {
    EventWindowRendererSDL<EC> m_ewrs;
  public:
    UlamContextRestrictedSDL(EventWindowRendererSDL<EC>& ewrs, const Tile<EC> & tile)
      : UlamContextRestricted<EC>(tile.GetElementTable(), tile.GetUlamClassRegistry())
      , m_ewrs(ewrs)
    { }

    virtual bool HasEventWindowRenderer() const { return true; }
    virtual const EventWindowRenderer<EC> & GetEventWindowRenderer() const {
      return m_ewrs;
    }
  };
} /* namespace MFM */

#endif /*DRAWABLESDL_H*/
