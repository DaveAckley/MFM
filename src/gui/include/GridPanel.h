/*                                              -*- mode:C++ -*-
  GridPanel.h Panel for rendering a Grid
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
  \file GridPanel.h Panel for rendering a Grid
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef GRIDPANEL_H
#define GRIDPANEL_H

#include "AtomViewPanel.h"
#include "itype.h"
#include "MDist.h"
#include "Panel.h"
#include "Sense.h"
#include "TileRenderer.h"
#include "Util.h"
#include <math.h> /* for sqrt */
#include "GUIConstants.h"

namespace MFM
{
  template <class GC> class GridTool;    // FORWARD

  /**
   * A template class for displaying the Grid in a Panel.
   */
  template <class GC>
  class GridPanel : public Panel
  {
    typedef Panel Super;
   public:
    // Extract short type names
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    enum { R = EC::EVENT_WINDOW_RADIUS };

    typedef Grid<GC> OurGrid;
    typedef Tile<EC> OurTile;
    typedef Site<AC> OurSite;
    typedef TileRenderer<EC> OurTileRenderer;
    typedef GridTool<GC> OurGridTool;
    typedef AtomViewPanel<GC> OurAtomViewPanel;

    bool LoadDetails(const char * key, LineCountingByteSource & source)
    {
      if (Super::LoadDetails(key, source)) return true;
      if (this->GetTileRenderer().TileRendererLoadDetails(key, source)) return true;

      if (!strcmp("gpog",key))
      {
        SPointSerializer sp(m_gridOriginDit);
        return 1 == source.Scanf("%@",&sp);
      }

      if (!strcmp("tslr",key))
      {
        u32 doit;
        if (1 != source.Scanf("%d", &doit) || !doit)
          return false;
        this->UnselectAllTiles();
        return true;
      }

      {
        CharBufferByteSource cbbs(key, strlen(key));
        u32 rowidx;
        if (5 == cbbs.Scanf("tsli%D",&rowidx))
        {
          if ((rowidx % 32) != 0) return false;
          if (rowidx + 32 >= MAX_TILES_IN_GRID) return false;

          u32 selbits;
          if (1 != source.Scanf("%08x)", &selbits))
            return false;

          m_selectedTiles.Write(rowidx, 32, selbits);
          return true;
        }
      }

      return false;
    }

    virtual void SaveDetails(ByteSink & sink) const
    {
      Super::SaveDetails(sink);
      this->GetTileRenderer().TileRendererSaveDetails(sink);

      {
        SPoint tmp(m_gridOriginDit);
        SPointSerializer sp(tmp);
        sink.Printf(" PP(gpog=%@)\n", &sp);
      }

      {
        sink.Printf(" PP(tslr=1)\n");
        for (u32 i = 0; i < MAX_TILES_IN_GRID; i += 32)
        {
          u32 selbits = m_selectedTiles.Read(i, 32);
          if (selbits != 0)
            sink.Printf(" PP(tsli%D=%08x)\n", i, selbits);
        }
      }
    }

    OurTileRenderer & GetTileRenderer()
    {
      MFM_API_ASSERT_NONNULL(m_tileRenderer);
      return *m_tileRenderer;
    }

    const OurTileRenderer & GetTileRenderer() const
    {
      MFM_API_ASSERT_NONNULL(m_tileRenderer);
      return *m_tileRenderer;
    }

    void SetTileRenderer(OurTileRenderer * tr)
    {
      MFM_API_ASSERT_NONNULL(tr);
      if (m_tileRenderer) FAIL(ILLEGAL_STATE);
      m_tileRenderer = tr;
    }

    u32 GetAtomDit() const { return GetTileRenderer().GetAtomSizeDit(); }

    void PaintSelectedTileMarkers(Drawing & drawing)
    {
      OurGrid & grid = GetGrid();
      drawing.SetForeground(Drawing::YELLOW);
      for (u32 tx = 0; tx < grid.GetWidth(); ++tx)
      {
        for (u32 ty = 0; ty < grid.GetHeight(); ++ty)
        {
          UPoint utc(tx,ty);
          if (IsTileSelected(utc))
          {
            SPoint tc(tx,ty); // sigh
            Rect rdit = MapTileInGridToScreenDit(grid.GetTile(tc), tc);
            drawing.DrawRectDit(rdit);
          }
        }
      }
    }

    void TogglePauseOnSelectedTiles()
    {
      OurGrid & grid = GetGrid();
      for (u32 tx = 0; tx < grid.GetWidth(); ++tx)
      {
        for (u32 ty = 0; ty < grid.GetHeight(); ++ty)
        {
          SPoint tc(tx,ty);
          if (IsTileSelected(MakeUnsigned(tc)))
            grid.SetTileEnabled(tc, !grid.IsTileEnabled(tc));
        }
      }
    }

    void ClearSelectedTiles()
    {
      OurGrid & grid = GetGrid();
      for (u32 tx = 0; tx < grid.GetWidth(); ++tx)
      {
        for (u32 ty = 0; ty < grid.GetHeight(); ++ty)
        {
          SPoint tc(tx,ty);
          if (IsTileSelected(MakeUnsigned(tc)))
            grid.EmptyTile(tc);
        }
      }
    }

    bool IsTileSelected(UPoint positionInGrid)
    {
      return m_selectedTiles.ReadBit(ToTileSelectIndex(positionInGrid));
    }

    void SetTileSelected(UPoint positionInGrid, bool selected)
    {
      m_selectedTiles.WriteBit(ToTileSelectIndex(positionInGrid), selected);
    }

    void UnselectAllTiles()
    {
      m_selectedTiles.Clear();
    }

   private:
    OurTileRenderer * m_tileRenderer;
    void SetAtomDit(u32 newdit) { GetTileRenderer().SetAtomSizeDit(newdit); }

    OurGrid* m_mainGrid;
    OurGridTool* m_currentGridTool;
    SPoint m_gridOriginDit;

    SPoint m_leftButtonDragStartPix;
    SPoint m_leftButtonGridStartDit;

    OurAtomViewPanel m_avp;

    enum {
      MAX_GRID_SIDE = 128,  // Yeah right.  More power to you..
      MAX_TILES_IN_GRID = MAX_GRID_SIDE * MAX_GRID_SIDE
    };

    BitVector<MAX_TILES_IN_GRID> m_selectedTiles;

    u32 ToTileSelectIndex(UPoint positionInGrid)
    {
      u32 x = positionInGrid.GetX(), y = positionInGrid.GetY();
      if (x >= MAX_GRID_SIDE || y >= MAX_GRID_SIDE)
        FAIL(ILLEGAL_ARGUMENT);
      return x*MAX_GRID_SIDE + y;
    }

    void ZoomAroundPix(SPoint aroundPix, u32 newAtomDrawDit)
    {
      if (newAtomDrawDit < OurTileRenderer::MINIMUM_ATOM_SIZE_DIT)
        newAtomDrawDit = OurTileRenderer::MINIMUM_ATOM_SIZE_DIT;

      if (newAtomDrawDit > OurTileRenderer::MAXIMUM_ATOM_SIZE_DIT)
        newAtomDrawDit = OurTileRenderer::MAXIMUM_ATOM_SIZE_DIT;

      SPoint aroundDit = aroundPix * Drawing::DIT_PER_PIX;
      double oldSize = GetAtomDit();
      double atomx =  (aroundDit.GetX() - m_gridOriginDit.GetX()) / oldSize;
      double atomy =  (aroundDit.GetY() - m_gridOriginDit.GetY()) / oldSize;

      SetAtomDit(newAtomDrawDit);

      double newAroundDitX = atomx * GetAtomDit() + m_gridOriginDit.GetX();
      double newAroundDitY = atomy * GetAtomDit() + m_gridOriginDit.GetY();

      double deltax = newAroundDitX - aroundDit.GetX();
      double deltay = newAroundDitY - aroundDit.GetY();
      m_gridOriginDit -= SPoint((s32) deltax, (s32) deltay);
    }

   public:
    OurAtomViewPanel & GetAtomViewPanel() {
      return m_avp;
    }

    GridPanel()
      : Super()
      , m_tileRenderer(0)
      , m_mainGrid(0)
      , m_currentGridTool(0)
      , m_gridOriginDit(0,0)
      , m_leftButtonDragStartPix(0,0)
      , m_leftButtonGridStartDit (0,0)
      , m_avp()
    {
      SetName("GridPanel");
      SetDimensions(SCREEN_INITIAL_WIDTH,
                    SCREEN_INITIAL_HEIGHT);
      SetRenderPoint(SPoint(0, 0));
      SetForeground(Drawing::BLACK);
      SetBackground(Drawing::BLACK);
      //      SetBackground(Drawing::ORANGE);
      m_avp.SetVisible(false);
      m_avp.SetRenderPoint(SPoint(300,30));
      this->Insert(&m_avp, 0);
    }

    void Init()
    {
    }

    void SetGrid(OurGrid* mainGrid)
    {
      m_mainGrid = mainGrid;
    }

    OurGrid & GetGrid()
    {
      MFM_API_ASSERT_NONNULL(m_mainGrid);
      return * m_mainGrid;
    }

    const OurGrid & GetGrid() const
    {
      MFM_API_ASSERT_NONNULL(m_mainGrid);
      return * m_mainGrid;
    }

    void SetGridTool(OurGridTool * gridTool)
    {
      m_currentGridTool = gridTool;
    }

    OurGridTool * GetGridTool()
    {
      return m_currentGridTool;
    }

    Rect MapTileInGridToScreenDit(const OurTile & tile, const SPoint tileCoord) const
    {
      u32 atomDit = GetAtomDit();
      bool caches = GetTileRenderer().IsDrawCaches();
      u32 sizeDit;
      u32 spacingDit;
      if (caches)
      {
        sizeDit = tile.TILE_SIDE * atomDit;
        spacingDit = sizeDit + atomDit / 2;
      }
      else
      {
        sizeDit = tile.OWNED_SIDE * atomDit;
        spacingDit = sizeDit;
      }
      return Rect(tileCoord * spacingDit + m_gridOriginDit, UPoint(sizeDit,sizeDit));

    }

#if 0
    OurSite * GetSiteAtScreenDit(const SPoint screenDit, bool includeCaches)
    {
      bool cachesDrawn = GetTileRenderer().IsDrawCaches();
      u32 atomDit = GetAtomDit();
      for (typename Grid<GC>::iterator_type i = m_mainGrid->begin(); i != m_mainGrid->end(); ++i)
      {
        OurTile & tile = *i;
        SPoint tileCoord = i.At();
        const Rect screenRectForTileDit = MapTileInGridToScreenDit(tile, tileCoord);
        if (screenRectForTileDit.Contains(screenDit))
        {
          SPoint siteCoord = (screenDit - screenRectForTileDit.GetPosition()) / atomDit;
          if (cachesDrawn)
          {
            if (!includeCaches && tile.RegionIn(siteCoord) == OurTile::REGION_CACHE)
              return 0;
            return &tile.GetSite(siteCoord);
          }
          else
          {
            if (tile.IsInUncachedTile(siteCoord))
              return &tile.GetUncachedSite(siteCoord);
            return 0;
          }
        }
      }

      return 0;
    }
#endif

    OurSite * GetSiteAtGridCoord(const UPoint gridCoord)
    {
      FAIL(INCOMPLETE_CODE);
    }

    bool GetGridCoordAtScreenDit(const SPoint screenDit, UPoint & gridCoord)
    {
      bool cachesDrawn = GetTileRenderer().IsDrawCaches();
      u32 atomDit = GetAtomDit();
      for (typename Grid<GC>::iterator_type i = m_mainGrid->begin(); i != m_mainGrid->end(); ++i)
      {
        OurTile & tile = *i;
        SPoint tileCoord = i.At();
        const Rect screenRectForTileDit = MapTileInGridToScreenDit(tile, tileCoord);
        if (screenRectForTileDit.Contains(screenDit))
        {
          const SPoint siteInTileCoord = (screenDit - screenRectForTileDit.GetPosition()) / atomDit;
          SPoint siteInGridCoord;
          if (cachesDrawn)
          {
            if (tile.RegionIn(siteInTileCoord) == OurTile::REGION_CACHE)
            {
              Dir dir = tile.CacheAt(siteInTileCoord);
              if (dir < 0) FAIL(ILLEGAL_STATE);
              SPoint offset;
              Dirs::FillDir(offset, dir);
              SPoint otherTileCoord = tileCoord + offset;
              if (!m_mainGrid->IsLegalTileIndex(otherTileCoord))
                return false;

              // Hmm, is it this mysterious code again?  (Grid.tcc:404)
              SPoint siteInOtherTile = siteInTileCoord - offset*tile.OWNED_SIDE;
              siteInGridCoord = otherTileCoord * tile.OWNED_SIDE + OurTile::TileCoordToOwned(siteInOtherTile);
            }
            else
            {
              siteInGridCoord = tileCoord * tile.OWNED_SIDE + OurTile::TileCoordToOwned(siteInTileCoord);
            }
          }
          else
          {
            siteInGridCoord = tileCoord * tile.OWNED_SIDE + siteInTileCoord;
          }
          gridCoord = MakeUnsigned(siteInGridCoord);
          return true;
        }
      }

      return false;
    }

    bool GetScreenRectDitOfGridCoord(const UPoint & gridCoord, Rect & screenRectDit)
    {
      SPoint tileInGrid;
      SPoint siteInTile;
      if (!m_mainGrid->MapGridToTile(MakeSigned(gridCoord), tileInGrid, siteInTile))
        return false;
      OurTile & tile = m_mainGrid->GetTile(tileInGrid);
      const Rect screenRectForTileDit = MapTileInGridToScreenDit(tile, tileInGrid);
      u32 atomDit = this->GetAtomDit();
      bool caches = GetTileRenderer().IsDrawCaches();
      if (!caches)
        siteInTile -= SPoint(R,R);
      screenRectDit =
        Rect(siteInTile * atomDit + screenRectForTileDit.GetPosition(),
             UPoint(atomDit, atomDit));
      return true;
    }


    void PaintTiles(Drawing & drawing)
    {
      GetTileRenderer().SetDrawBases(m_currentGridTool && m_currentGridTool->IsSiteEdit());
      for (typename Grid<GC>::iterator_type i = m_mainGrid->begin(); i != m_mainGrid->end(); ++i)
      {
        SPoint tileCoord = i.At();
        Rect screenDitForTile = MapTileInGridToScreenDit(*i,tileCoord);
        GetTileRenderer().PaintTileAtDit(drawing, screenDitForTile.GetPosition(), *i);
      }
    }

    void PaintGridOverlays(Drawing & drawing)
    {
      if (m_currentGridTool)
        m_currentGridTool->PaintOverlay(drawing);
      PaintSelectedTileMarkers(drawing);
    }


   public:

    virtual void PaintComponent(Drawing& drawing)
    {
      this->Panel::PaintComponent(drawing);

      this->PaintTiles(drawing);

      this->PaintGridOverlays(drawing);
    }

    static bool IsLeftOrRightSetInButtonMask(u32 mask)
    {
      return mask & ((1 << SDL_BUTTON_LEFT)|(1 << SDL_BUTTON_RIGHT));
    }

    static bool IsLeftSetInButtonMask(u32 mask)
    {
      return mask & (1 << SDL_BUTTON_LEFT);
    }

    static bool IsRightSetInButtonMask(u32 mask)
    {
      return mask & (1 << SDL_BUTTON_RIGHT);
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;
      if(event.type == SDL_MOUSEBUTTONUP)
      {
        if (m_currentGridTool)
        {
          m_currentGridTool->Release(mbe);
          return true;
        }
      }

      if(event.type == SDL_MOUSEBUTTONDOWN)
      {
        if (m_currentGridTool)
        {
          if(IsLeftOrRightSetInButtonMask(1<<event.button) &&
             (mbe.m_keyboardModifiers == 0))
          {
            m_currentGridTool->Press(mbe);
            return true;
          }
        }

        if (event.button == SDL_BUTTON_LEFT && !m_currentGridTool)
        {
          UPoint gridCoord;
          if (this->GetGridCoordAtScreenDit(Drawing::MapPixToDit(mbe.GetAt()), gridCoord))
          {
            typename Grid<GC>::GridTouchEvent gte;
            gte.m_touchType = TOUCH_TYPE_HEAVY;
            gte.m_gridAtomCoord = MakeSigned(gridCoord);
            m_mainGrid->SenseTouchAround(gte);
          }
        }

        SPoint hitAtScreenPix = GetAbsoluteLocation();
        hitAtScreenPix.Set(event.x - hitAtScreenPix.GetX(),
                           event.y - hitAtScreenPix.GetY());

        switch (event.button)
        {
        case SDL_BUTTON_LEFT:
          m_leftButtonDragStartPix = hitAtScreenPix;
          m_leftButtonGridStartDit = m_gridOriginDit;
          return true;
        default:
          LOG.Message("MBE %d,%d + %d",
                      mbe.m_event.button.x,
                      mbe.m_event.button.y,
                      mbe.m_keyboardModifiers);
          break;
        case SDL_BUTTON_WHEELUP:
          {
            u32 newDit;
            if (mbe.m_keyboardModifiers & KMOD_SHIFT)
              newDit = GetAtomDit() + 1;
            else
              newDit = 17*GetAtomDit()/16;
            ZoomAroundPix(hitAtScreenPix, newDit);
            break;
          }
        case SDL_BUTTON_WHEELDOWN:
          {
            u32 newDit;
            if (GetAtomDit() > Drawing::DIT_PER_PIX)
            {
              if (mbe.m_keyboardModifiers & KMOD_SHIFT)
                newDit = GetAtomDit() - 1;
              else
                newDit = 15*GetAtomDit()/16;
              ZoomAroundPix(hitAtScreenPix, newDit);
            }
            break;
          }
        }
      }
      return false;
    }

    virtual bool Handle(MouseMotionEvent& mme)
    {
      SDL_MouseMotionEvent & event = mme.m_event.motion;
      if (mme.m_keyboardModifiers & KMOD_CTRL)
      {
        if(mme.m_buttonMask & (1 << SDL_BUTTON_LEFT))
        {
          SPoint nowAt(event.x, event.y);
          SPoint delta = nowAt - m_leftButtonDragStartPix;
          m_gridOriginDit = m_leftButtonGridStartDit + Drawing::MapPixToDit(delta);
        }
        return true;
      }
      if (m_currentGridTool)
      {
        if(/*IsLeftOrRightSetInButtonMask(mme.m_buttonMask) && */
           (mme.m_keyboardModifiers == 0))
        {
          m_currentGridTool->Drag(mme);
          return true;
        }
      }
      else
      {
        UPoint gridCoord;
        if (this->GetGridCoordAtScreenDit(Drawing::MapPixToDit(mme.GetAt()), gridCoord))
        {
          typename Grid<GC>::GridTouchEvent gte;
          if (mme.m_buttonMask == 0)
            gte.m_touchType = TOUCH_TYPE_PROXIMITY;
          else
            gte.m_touchType = TOUCH_TYPE_LIGHT;
          gte.m_gridAtomCoord = MakeSigned(gridCoord);
          m_mainGrid->SenseTouchAround(gte);
          return true;
        }
      }

      {
        LOG.Debug("MME %d,%d + %d",
                  mme.m_event.motion.x,
                  mme.m_event.motion.y,
                  mme.m_keyboardModifiers);
      }
      return false;
    }
  };
} /* namespace MFM */

#endif /* GRIDPANEL_H */
