/*                                              -*- mode:C++ -*-
  T2RootPanel.h T2Viz root panel
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
  \file T2RootPanel.h T2Viz root panel
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef T2ROOTPANEL_H
#define T2ROOTPANEL_H

#include "itype.h"
#include "Panel.h"
#include "Drawing.h"
#include "Fail.h"
#include "SDL.h"
#include "Menu.h"

namespace MFM
{
  /**
   * The root panel for T2Viz
   */
  class T2RootPanel : public MenuMakerPanel
  {
    typedef MenuMakerPanel Super;

    virtual void addItems(Menu& menu) ;

  private:

    /**
     * Initializes this T2RootPanel
     */
    void Init() { }

  public:

    T2RootPanel() { }

    /**
     * Destructor.
     */
    ~T2RootPanel() { }

    /**
     * Sets the location of this T2RootPanel, relative to the Panel
     * which it is located inside of, at which to render this
     * T2RootPanel and process mouse hits.
     *
     * @param location The new location that this T2RootPanel will
     *                 reside at.
     */
    void SetLocation(const SPoint& location)
    {
      SetRenderPoint(location);
    }

    virtual void HandleResize(const UPoint& parentSize)
    {
      /* T2RootPanels don't resize automatically? */
    }

  };
}

#endif /* T2ROOTPANEL_H */
