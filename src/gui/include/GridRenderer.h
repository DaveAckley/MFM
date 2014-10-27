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
#include "AtomViewPanel.h"

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

    UPoint m_dimensions;

    SPoint m_selectedTile;

    /**
     * The coordinates, ignoring all caches, of the Atom that is
     * selected. Therefore, (0, 0) is the top-left tile's top-left
     * shared atom and (0, TILE_WIDTH) is the tile at (0, 1)'s
     * top-left shared atom. If either coordinate is negative, treat
     * as if no Atom is selected.
     */
    SPoint m_selectedAtom;

    SPoint m_hoveredAtom;

    SPoint m_cloneOrigin;

    static const EventWindowRenderMode m_defaultRenderMode =
      EVENTWINDOW_RENDER_OFF;

    static const bool m_renderTilesSeparatedDefault = false;

    bool m_renderTilesSeparated;

    EventWindowRenderMode m_currentEWRenderMode;

   public:

    GridRenderer(TileRenderer* tr);

    GridRenderer();

    ~GridRenderer();

    TileRenderer& GetTileRenderer()
    { return m_tileRenderer; }

    bool IsRenderingTilesSeparated()
    { return m_renderTilesSeparated; }

    void SetEventWindowRenderMode(EventWindowRenderMode mode);

    UPoint GetDimensions() const;

    void SetDimensions(UPoint dimensions);

    void IncreaseAtomSize(SPoint around = SPoint(0,0));

    void ToggleTileSeparation();

    void ToggleDataHeatmap();

    void DecreaseAtomSize(SPoint around = SPoint(0,0));

    void ToggleGrid();

    void ToggleDrawAtomsAsSquares()
    {
      m_tileRenderer.ToggleDrawAtomsAsSquares();
    }

    bool* GetGridEnabledPointer()
    {
      return m_tileRenderer.GetGridEnabledPointer();
    }

    bool* GetDrawDataHeatPointer()
    {
      return m_tileRenderer.GetDrawDataHeatPointer();
    }

    void ToggleMemDraw();

    const SPoint & GetDrawOrigin() const ;

    void SetDrawOrigin(const SPoint & origin) ;

    void SetCloneOrigin(const SPoint& cloneOrigin) ;

    void MoveUp(u8 amount);

    void MoveDown(u8 amount);

    void MoveLeft(u8 amount);

    void MoveRight(u8 amount);

    void DeselectTile();

    void DeselectAtom();

    void DeselectHoveredAtom()
    {
      m_hoveredAtom.Set(-1, -1);
    }

    SPoint GetSelectedTile() const;

    SPoint GetSelectedAtom() const
    {
      return m_selectedAtom;
    }

    SPoint GetHoveredAtom() const
    {
      return m_hoveredAtom;
    }

    template <class GC>
    void RenderGrid(Drawing & drawing, Grid<GC>& grid, u32 brushSize);

    template <class GC>
    void SelectTile(Grid<GC>& grid, SPoint clickPt);

    template <class GC>
    void SelectAtom(Grid<GC>& grid, SPoint clickPt);

    template <class GC>
    void SetHoveredAtom(Grid<GC>& grid, SPoint clickPt);

  };
} /* namespace MFM */
#include "GridRenderer.tcc"

#endif /*GRIDRENDERER_H*/
