/*                                              -*- mode:C++ -*-
  TileRenderer.h Code and configuration for rendering tiles
  Copyright (C) 2014-2016 The Regents of the University of New Mexico.  All rights reserved.

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
  \file TileRenderer.h Code and configuration for rendering tiles
  \author Dave Ackley
  \author Trent R. Small.
  \date (C) 2014-2016 All rights reserved.
  \lgpl
 */
#ifndef TILERENDERER_H
#define TILERENDERER_H

#include "Tile.h"
#include "Site.h"
#include "Drawing.h"

namespace MFM
{
  template  <class EC>
  class TileRenderer
  {
  public:

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    typedef typename EC::SITE S;
    typedef Tile<EC> OurTile;
    typedef Site<AC> OurSite;

    enum { EWR = EC::EVENT_WINDOW_RADIUS };

    enum {
      MINIMUM_ATOM_SIZE_DIT =    1 * Drawing::DIT_PER_PIX,
      DEFAULT_ATOM_SIZE_DIT =   16 * Drawing::DIT_PER_PIX,
      MAXIMUM_ATOM_SIZE_DIT = 1024 * Drawing::DIT_PER_PIX
    };

    enum DrawSiteType {
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
      DRAW_SITE_TYPE_COUNT
    };

    enum DrawSiteShape {
      DRAW_SHAPE_FILL,           //< Flood fill site entirely (square)
      DRAW_SHAPE_CIRCLE,         //< Draw circle touching site edges
      DRAW_SHAPE_CDOT,           //< Draw small centered dot
      DRAW_SHAPE_COUNT
    };

    bool TileRendererLoadDetails(const char * key, LineCountingByteSource & source) ;

    void TileRendererSaveDetails(ByteSink & sink) const ;

    const char * GetDrawBackgroundTypeName() const
    {
      return GetDrawSiteTypeName(m_drawBackgroundType);
    }

    const char * GetDrawMidgroundTypeName() const
    {
      return GetDrawSiteTypeName(m_drawMidgroundType);
    }

    const char * GetDrawForegroundTypeName() const
    {
      return GetDrawSiteTypeName(m_drawForegroundType);
    }

    static const char * GetDrawSiteTypeName(DrawSiteType t) ;

    /**
       How much space will it currently take to draw this whole tile?
     */
    SPoint ComputeDrawSizeDit(const OurTile & tile) const
    {
      return ComputeDrawSizeDit(tile, m_drawCacheSites ? OurTile::REGION_CACHE : OurTile::REGION_SHARED);
    }

    SPoint ComputeDrawSizeDit(const OurTile & tile, u32 tileRegion) const;

    /**
       Is this region currently being drawn at all?
     */
    bool IsRegionDrawn(const OurTile & tile, u32 tileRegion) const
    {
      if (m_drawCacheSites) return tileRegion >= OurTile::REGION_CACHE;
      return tileRegion >= OurTile::REGION_SHARED;
    }

    /**
       Where should this region be drawn inside the whole tile,
       assuming the tile top-left is (0,0)?  FAILs if the region
       should not be drawn at all
     */
    SPoint ComputeDrawInsetDit(const OurTile & tile, u32 tileRegion) const
    {
      u32 outer  = m_drawCacheSites ? OurTile::REGION_CACHE : OurTile::REGION_SHARED;
      if (tileRegion < outer) FAIL(ILLEGAL_ARGUMENT);
      u32 ditsIn = m_atomSizeDit * EWR * (tileRegion - outer);
      return SPoint(ditsIn, ditsIn);
    }

    TileRenderer();

    void PaintTileAtDit(Drawing & drawing,
                        const SPoint ditOrigin, const OurTile & tile) ;

    void PaintSites(Drawing & drawing,
                    const DrawSiteType drawType, const DrawSiteShape shape,
                    const SPoint ditOrigin, const OurTile & tile) ;

    void PaintSiteAtDit(Drawing & drawing,
                        const DrawSiteType drawType, const DrawSiteShape shape,
                        const SPoint ditOrigin, const OurSite & site, const OurTile & inTile) ;

    void PaintShapeForSite(Drawing & drawing, const DrawSiteShape shape, const SPoint ditOrigin, u32 color);

    void PaintBadAtomAtDit(Drawing & drawing, const SPoint ditOrigin) ;

    void PaintUnderlays(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile) ;

    void PaintOverlays(Drawing & drawing, const SPoint ditOrigin, const OurTile & tile) ;

    void PaintTileHistoryInfo(Drawing & drawing, const SPoint ditOrigin, const OurTile & tile) ;

    void OutlineEventWindowInTile(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile, SPoint site, u32 color) ;

    bool IsDrawGrid() const
    {
      return m_drawGridLines;
    }

    void SetDrawGrid(bool value)
    {
      m_drawGridLines = value;
    }

    bool IsDrawCaches() const
    {
      return m_drawCacheSites;
    }

    void SetDrawCaches(bool value)
    {
      m_drawCacheSites = value;
    }

    bool IsDrawBases() const
    {
      return m_drawBases;
    }

    void SetDrawBases(bool value)
    {
      m_drawBases = value;
    }

    u32 NextDrawBackgroundType()
    {
      return m_drawBackgroundType = (DrawSiteType) ((m_drawBackgroundType + 1) % DRAW_SITE_TYPE_COUNT);
    }

    u32 NextDrawMidgroundType()
    {
      return m_drawMidgroundType = (DrawSiteType) ((m_drawMidgroundType + 1) % DRAW_SITE_TYPE_COUNT);
    }

    u32 NextDrawForegroundType()
    {
      return m_drawForegroundType = (DrawSiteType) ((m_drawForegroundType + 1) % DRAW_SITE_TYPE_COUNT);
    }

    u32 GetAtomSizeDit() const
    {
      return m_atomSizeDit;
    }

    void SetAtomSizeDit(u32 newdit)
    {
      if (newdit==0) FAIL(ILLEGAL_ARGUMENT);
      m_atomSizeDit = newdit;
    }

  private:
    static bool IsDrawBase(DrawSiteType t)
    {
      return t >= DRAW_SITE_BASE && t <= DRAW_SITE_BASE_2;
    }

    bool IsBaseVisible()
    {
      return
        IsDrawBase(m_drawBackgroundType) ||
        IsDrawBase(m_drawMidgroundType) ||
        IsDrawBase(m_drawForegroundType);
    }

    DrawSiteType m_drawBackgroundType;
    DrawSiteType m_drawMidgroundType;
    DrawSiteType m_drawForegroundType;

    bool m_drawEventWindow;

    bool m_drawGridLines;

    bool m_drawCacheSites;

    bool m_drawBases;

    u32 m_atomSizeDit;

    u32 m_gridLineColor;

    u32 m_regionColors[Tile<EC>::REGION_COUNT];

    /* XXX
    u32 m_selectedHiddenColor;
    u32 m_selectedPausedColor;
    */


  };
} /* namespace MFM */

#include "TileRenderer.tcc"

#endif /*TILERENDERER_H*/
