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
    enum DrawBackgroundType {
      DRAW_BACKGROUND_LIGHT_TILE,       //< Light grey rendering of tile regions
      DRAW_BACKGROUND_NONE,             //< No background rendering
      DRAW_BACKGROUND_DARK_TILE,        //< Dark grey rendering of hidden regions
      DRAW_BACKGROUND_CHANGE_AGE,       //< CubeHelix rendering of events-since-change
      DRAW_BACKGROUND_SITE,             //< Last color painted on site
      DRAW_BACKGROUND_TYPE_COUNT
    } m_drawBackgroundType;

    enum DrawForegroundType {
      DRAW_FOREGROUND_ELEMENT,  //< Hardcoded physics color
      DRAW_FOREGROUND_ATOM_1,   //< Dynamic per-atom rendering type 1
      DRAW_FOREGROUND_ATOM_2,   //< Dynamic per-atom rendering type 2
      DRAW_FOREGROUND_ATOM_3,   //< Dynamic per-atom rendering type 3
      DRAW_FOREGROUND_SITE,     //< Last color painted on site
      DRAW_FOREGROUND_NONE,     //< Do not draw atoms at all
      DRAW_FOREGROUND_TYPE_COUNT
    } m_drawForegroundType;


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

    UPoint m_dimensions;

    template <class EC>
    void RenderMemRegions(Drawing & drawing, SPoint& pt,
                          bool renderCache, bool selected, bool lowlight,
                          const u32 TILE_SIDE);

    template <class EC>
    void RenderVisibleRegionOutlines(Drawing & drawing, SPoint& pt, bool renderCache,
                                     bool selected, bool lowlight,
                                     const u32 TILE_SIDE);

    template <class EC>
    void RenderMemRegion(Drawing & drawing, SPoint& pt, int regID,
                         u32 color, bool renderCache,
                         const u32 TILE_SIDE);

    template <class EC>
    void RenderGrid(Drawing & drawing, SPoint* pt, bool renderCache,
                    const u32 TILE_SIDE);

    void RenderAtomBG(Drawing & drawing, SPoint& offset, Point<int>& atomloc,
                      u32 color);

    template <class EC>
    void RenderAtoms(Drawing & drawing, SPoint& pt, Tile<EC>& tile,
                     bool renderCache, bool lowlight);

    template <class EC>
    void RenderAtom(Drawing & drawing, const SPoint& atomLoc, const UPoint& rendPt,
                    Tile<EC>& tile, bool lowlight);

    template <class EC>
    void RenderBadAtom(Drawing& drawing, const UPoint& rendPt);

    template <class EC>
    u32 GetSiteColor(Tile<EC>& tile, const Site<typename EC::ATOM_CONFIG> & site, u32 selector = 0);

#if 0
    template <class EC>
    u32 GetDataHeatColor(Tile<EC>& tile, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom);
#endif

    template <class EC>
    void RenderEventWindow(Drawing & drawing, SPoint& offset, Tile<EC>& tile, bool renderCache);

  public:
    bool LoadDetails(LineCountingByteSource & source) ;

    void SaveDetails(ByteSink & sink) const ;

    const char * GetDrawBackgroundTypeName() const;

    const char * GetDrawForegroundTypeName() const;

    TileRenderer();

    template <class EC>
    void RenderTile(Drawing & drawing, Tile<EC>& t, SPoint& loc, bool renderWindow,
                    bool renderCache, bool selected, SPoint* selectedAtom, SPoint* cloneOrigin);

    void SetDimensions(UPoint dimensions)
    {
      m_dimensions = dimensions;
    }

    UPoint GetDimensions() const
    {
      return m_dimensions;
    }

    void ToggleDrawAtomsAsSquares()
    {
      m_renderSquares = !m_renderSquares;
    }

    void SetDrawGrid(bool draw)
    {
      m_drawGrid = draw;
    }

    bool IsDrawGrid() const
    {
      return m_drawGrid;
    }

#if 0
    bool* GetDrawDataHeatPointer()
    {
      return &m_drawDataHeat;
    }
#endif

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

    DrawForegroundType NextDrawForegroundType()
    {
      return
        m_drawForegroundType =
        (DrawForegroundType) ((m_drawForegroundType + 1) % DRAW_FOREGROUND_TYPE_COUNT);
    }

    u32 GetDrawForegroundType()
    {
      return m_drawForegroundType;
    }

    void ChangeAtomSize(bool increase, SPoint around) ;

    u32 GetAtomSize()
    {
      return m_atomDrawSize;
    }

    void ToggleGrid();

    u32 NextDrawBackgroundType();

    void ToggleDataHeat();

    void Move(SPoint amount);

    void MoveUp(u8 amount);

    void MoveDown(u8 amount);

    void MoveLeft(u8 amount);

    void MoveRight(u8 amount);
  };
} /* namespace MFM */

#include "TileRenderer.tcc"

#endif /*TILERENDERER_H*/
