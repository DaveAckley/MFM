/*                                              -*- mode:C++ -*-
  GridRenderer.h Structure for rendering a Grid to an SDL_Surface
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
  \file GridRenderer.h Structure for rendering a Grid to an SDL_Surface
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include "Grid.h"
#include "Drawing.h"
#include "TileRenderer.h"

namespace MFM
{

  typedef enum
  {
    EVENTWINDOW_RENDER_OFF     = 0,
    EVENTWINDOW_RENDER_CURRENT = 1,
    EVENTWINDOW_RENDER_ALL     = 2
  } EventWindowRenderMode;


  class GridRenderer
  {
  private:

    TileRenderer m_tileRenderer;

    Point<u32> m_dimensions;

    SPoint m_selectedTile;

    static const EventWindowRenderMode m_defaultRenderMode =
      EVENTWINDOW_RENDER_OFF;

    static const bool m_renderTilesSeparatedDefault = false;

    bool m_renderTilesSeparated;

    EventWindowRenderMode m_currentEWRenderMode;

  public:

    GridRenderer(TileRenderer* tr);

    GridRenderer();

    ~GridRenderer();

    void SetEventWindowRenderMode(EventWindowRenderMode mode);

    UPoint& GetDimensions();

    void SetDimensions(Point<u32> dimensions);

    void IncreaseAtomSize();

    void ToggleTileSeparation();

    void ToggleDataHeatmap();

    void DecreaseAtomSize();

    void ToggleGrid();

    void ToggleMemDraw();

    void MoveUp(u8 amount);

    void MoveDown(u8 amount);

    void MoveLeft(u8 amount);

    void MoveRight(u8 amount);

    void DeselectTile();

    SPoint& GetSelectedTile();

    template <class GC>
    void RenderGrid(Drawing & drawing, Grid<GC>& grid);

    template <class GC>
    void SelectTile(Grid<GC>& grid, SPoint& clickPt);

  };
} /* namespace MFM */
#include "GridRenderer.tcc"

#endif /*GRIDRENDERER_H*/
