/*                                              -*- mode:C++ -*-
  TileRenderer.h SDL_Surface renderer for the Tile structure
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
  \file TileRenderer.h SDL_Surface renderer for the Tile structure
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef TILERENDERER_H
#define TILERENDERER_H

#include "Drawing.h"
#include "ElementTable.h"
#include "Tile.h"
#include "Panel.h"
#include "Point.h"
#include "SDL.h"


namespace MFM
{
  class TileRenderer
  {
   private:
    bool m_drawGrid;
    enum DrawRegionType { FULL, NO, EDGE, AGE, AGE_ONLY, MAX} m_drawMemRegions;
    bool m_drawDataHeat;
    u32 m_atomDrawSize;

    bool m_renderSquares;

    u32 m_gridColor;

    u32 m_cacheColor;
    u32 m_sharedColor;
    u32 m_visibleColor;
    u32 m_hiddenColor;
    u32 m_selectedHiddenColor;
    u32 m_selectedPausedColor;

    SPoint m_windowTL;

    Point<u32> m_dimensions;

    template <class CC>
    void RenderMemRegions(Drawing & drawing, SPoint& pt,
                          bool renderCache, bool selected, bool lowlight);

    template <class CC>
    void RenderVisibleRegionOutlines(Drawing & drawing, SPoint& pt, bool renderCache,
                                     bool selected, bool lowlight);

    template <class CC>
    void RenderMemRegion(Drawing & drawing, SPoint& pt, int regID,
                         u32 color, bool renderCache);

    template <class CC>
    void RenderGrid(Drawing & drawing, SPoint* pt, bool renderCache);

    void RenderAtomBG(Drawing & drawing, SPoint& offset, Point<int>& atomloc,
                      u32 color);

    template <class CC>
    void RenderAtoms(Drawing & drawing, SPoint& pt, Tile<CC>& tile,
                     bool renderCache, bool lowlight);

    template <class CC>
    void RenderAtom(Drawing & drawing, const SPoint& atomLoc, const UPoint& rendPt,
                    Tile<CC>& tile, bool lowlight);

    template <class CC>
    void RenderBadAtom(Drawing& drawing, const UPoint& rendPt);

    template <class CC>
    u32 GetAtomColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom, u32 selector = 0);

    template <class CC>
    u32 GetDataHeatColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom);

    template <class CC>
    void RenderEventWindow(Drawing & drawing, SPoint& offset, Tile<CC>& tile, bool renderCache);

  public:

    TileRenderer();

    template <class CC>
    void RenderTile(Drawing & drawing, Tile<CC>& t, SPoint& loc, bool renderWindow,
                    bool renderCache, bool selected, SPoint* selectedAtom, SPoint* cloneOrigin);

    void SetDimensions(Point<u32> dimensions)
    {
      m_dimensions = dimensions;
    }

    bool* GetGridEnabledPointer()
    {
      return &m_drawGrid;
    }

    void ToggleDrawAtomsAsSquares()
    {
      m_renderSquares = !m_renderSquares;
    }

    bool* GetDrawDataHeatPointer()
    {
      return &m_drawDataHeat;
    }

    const SPoint& GetWindowTL() const
    {
      return m_windowTL;
    }

    void SetWindowTL(const SPoint & newTL)
    {
      m_windowTL = newTL;
    }

    void IncreaseAtomSize(SPoint around)
    {
      ChangeAtomSize(true, around);
    }

    void DecreaseAtomSize(SPoint around)
    {
      ChangeAtomSize(false, around);
    }

    void ChangeAtomSize(bool increase, SPoint around) ;

    u32 GetAtomSize()
    {
      return m_atomDrawSize;
    }

    void ToggleGrid();

    void ToggleMemDraw();

    void ToggleDataHeat();

    void MoveUp(u8 amount);

    void MoveDown(u8 amount);

    void MoveLeft(u8 amount);

    void MoveRight(u8 amount);
  };
} /* namespace MFM */

#include "TileRenderer.tcc"

#endif /*TILERENDERER_H*/
