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
#include "ByteSource.h"
#include "ByteSink.h"

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

    //    UPoint m_dimensions;

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

    bool LoadDetails(LineCountingByteSource & source) ;

    void SaveDetails(ByteSink & sink) const ;

    GridRenderer(TileRenderer* tr);

    GridRenderer();

    ~GridRenderer();

    bool IsRenderingTilesSeparated()
    { return m_renderTilesSeparated; }

#if 0 // XXX KILLGREX
    u32 NextDrawForegroundType()
    {
      return m_tileRenderer.NextDrawForegroundType();
    }

    u32 GetDrawForegroundType()
    {
      return m_tileRenderer.GetDrawForegroundType();
    }

    const char * GetDrawForegroundTypeName() const
    {
      return m_tileRenderer.GetDrawForegroundTypeName();
    }

    const char * GetDrawBackgroundTypeName() const
    {
      return m_tileRenderer.GetDrawBackgroundTypeName();
    }
#endif

    void SetEventWindowRenderMode(EventWindowRenderMode mode);

// XXX KILLGREX    UPoint GetDimensions() const;

// XXX KILLGREX    void SetDimensions(UPoint dimensions);

// XXX KILLGREX    void IncreaseAtomSize(SPoint around = SPoint(0,0));

    void ToggleTileSeparation();

// XXX KILLGREX    void DecreaseAtomSize(SPoint around = SPoint(0,0));

// XXX KILLGREX    void ToggleGrid();

#if 0 // XXX KILLGREX
    void ToggleDrawAtomsAsSquares()
    {
      m_tileRenderer.ToggleDrawAtomsAsSquares();
    }

    void SetDrawGrid(bool draw)
    {
      m_tileRenderer.SetDrawGrid(draw);
    }

    bool IsDrawGrid() const
    {
      return m_tileRenderer.IsDrawGrid();
    }
#endif

    TileRenderer & GetTileRenderer() { return m_tileRenderer; }

    const TileRenderer & GetTileRenderer() const { return m_tileRenderer; }


#if 0
    bool* GetGridEnabledPointer()
    {
      return m_tileRenderer.GetGridEnabledPointer();
    }

    bool* GetDrawDataHeatPointer()
    {
      return m_tileRenderer.GetDrawDataHeatPointer();
    }
#endif

// XXX KILLGREX    u32 NextDrawBackgroundType();

// XXX KILLGREX    const SPoint & GetDrawOrigin() const ;

// XXX KILLGREX    void SetDrawOrigin(const SPoint & origin) ;

    void SetCloneOrigin(const SPoint& cloneOrigin) ;

// XXX KILLGREX    void Move(SPoint amount);

// XXX KILLGREX    void MoveUp(u8 amount);

// XXX KILLGREX    void MoveDown(u8 amount);

// XXX KILLGREX    void MoveLeft(u8 amount);

// XXX KILLGREX    void MoveRight(u8 amount);

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
