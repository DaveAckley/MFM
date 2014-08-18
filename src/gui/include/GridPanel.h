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
#include "GridRenderer.h"
#include "EditingTool.h"
#include "ToolboxPanel.h"
#include "Util.h"
#include <math.h> /* for sqrt */

#define SCREEN_INITIAL_WIDTH 1280
#define SCREEN_INITIAL_HEIGHT 1024

namespace MFM
{
  /**
   * A template class for displaying the Grid in a Panel.
   */
  template <class GC>
  class GridPanel : public Panel
  {
  public:
    // Extract short type names
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::PARAM_CONFIG P;
    typedef typename CC::ATOM_TYPE T;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};
    enum { TILE_SIDE_CACHE_SITES = P::TILE_WIDTH};
    enum { TILE_SIDE_LIVE_SITES = TILE_SIDE_CACHE_SITES - 2*R};

    static const u32 EVENT_WINDOW_RADIUS = R;
    static const u32 GRID_WIDTH_TILES = W;
    static const u32 GRID_HEIGHT_TILES = H;

    static const u32 GRID_WIDTH_LIVE_SITES = W * TILE_SIDE_LIVE_SITES;
    static const u32 GRID_HEIGHT_LIVE_TILES = H * TILE_SIDE_LIVE_SITES;

    typedef Grid<GC> OurGrid;

  private:
    GridRenderer* m_grend;
    OurGrid* m_mainGrid;
    ToolboxPanel<CC>* m_toolboxPanel;
    SPoint m_leftButtonDragStart;
    SPoint m_leftButtonGridStart;
    bool m_paintingEnabled;

    AtomViewPanel<GC> m_atomViewPanel;

   public:
    GridPanel() :
      m_paintingEnabled(false)
    {
      SetName("Grid Panel");
      SetDimensions(SCREEN_INITIAL_WIDTH,
                    SCREEN_INITIAL_HEIGHT);
      SetRenderPoint(SPoint(0, 0));
      SetForeground(Drawing::BLACK);
      SetBackground(Drawing::BLACK);

      m_grend = NULL;
      m_mainGrid = NULL;

      m_atomViewPanel.SetName("AtomViewer");
      m_atomViewPanel.SetRenderPoint(SPoint(326, 0));
      m_atomViewPanel.SetBackground(Drawing::BLACK);
      m_atomViewPanel.SetForeground(Drawing::GREY70);
      m_atomViewPanel.SetVisibility(true);

      Panel::Insert(&m_atomViewPanel, NULL);
    }

    AtomViewPanel<GC> * GetAtomViewPanel()
    {
      return &m_atomViewPanel;
    }

    void SetGrid(OurGrid* mainGrid)
    {
      m_mainGrid = mainGrid;
      m_atomViewPanel.SetGrid(m_mainGrid);
    }

    void SetGridRenderer(GridRenderer* grend)
    {
      m_grend = grend;
    }

    void SetToolboxPanel(ToolboxPanel<CC>* toolboxPanel)
    {
      m_toolboxPanel = toolboxPanel;
      m_atomViewPanel.SetToolboxPanel(m_toolboxPanel);
    }

    void SetPaintingEnabled(bool isPaintingEnabled)
    {
      m_paintingEnabled = isPaintingEnabled;
    }

    void DeselectAtomAndTile()
    {
      m_grend->DeselectTile();
      m_grend->DeselectAtom();
      m_atomViewPanel.SetAtom(NULL);
    }

  protected:
    virtual void PaintComponent(Drawing& drawing)
    {
      this->Panel::PaintComponent(drawing);

      m_grend->RenderGrid(drawing, *m_mainGrid);
    }

    void HandleSelectorTool(MouseButtonEvent& mbe)
    {
      SPoint pt = GetAbsoluteLocation();
      pt.Set(mbe.m_event.button.x - pt.GetX(),
             mbe.m_event.button.y - pt.GetY());

      m_grend->SelectTile(*m_mainGrid, pt);
    }

    void HandleAtomSelectorTool(MouseButtonEvent& mbe)
    {
      HandleAtomSelectorTool(mbe.m_event.button.button,
                             SPoint(mbe.m_event.button.x,
                                    mbe.m_event.button.y));
    }

    void HandlePencilTool(MouseButtonEvent& mbe)
    {
      HandlePencilTool(mbe.m_event.button.button,
                       SPoint(mbe.m_event.button.x,
                              mbe.m_event.button.y));
    }

    void HandleBrushTool(MouseButtonEvent& mbe)
    {
      HandleBrushTool(mbe.m_event.button.button,
                      SPoint(mbe.m_event.button.x,
                             mbe.m_event.button.y));
    }

    void HandleEraserTool(MouseButtonEvent& mbe)
    {
      HandleEraserTool(mbe.m_event.button.button,
                       SPoint(mbe.m_event.button.x,
                              mbe.m_event.button.y));
    }

    void HandleBucketTool(MouseButtonEvent& mbe)
    {
      HandleBucketTool(mbe.m_event.button.button,
                       SPoint(mbe.m_event.button.x,
                              mbe.m_event.button.y));
    }

    void HandleXRayTool(MouseButtonEvent& mbe)
    {
      HandleXRayTool(mbe.m_event.button.button,
                     SPoint(mbe.m_event.button.x,
                            mbe.m_event.button.y));
    }

    void HandleAtomSelectorTool(u8 button, SPoint clickPt)
    {
      SPoint pt = GetAbsoluteLocation();
      pt.Set(clickPt.GetX() - pt.GetX(),
             clickPt.GetY() - pt.GetY());

      m_grend->SelectAtom(*m_mainGrid, pt);
      SPoint selectedAtom = m_grend->GetSelectedAtom();
      m_atomViewPanel.SetAtom(m_mainGrid->GetAtom(selectedAtom));
    }

    void HandlePencilTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintMapper(button, clickPt, 0, atom, false);
    }

    void HandleBrushTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintMapper(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(), atom, false);
    }

    void HandleEraserTool(u8 button, SPoint clickPt)
    {
      PaintMapper(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(),
                  Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), false);
    }

    void HandleBucketTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      /* Filling empty results in stack overflow for obvious reasons */
      if(!Atom<CC>::IsType(atom, Element_Empty<CC>::THE_INSTANCE.GetType()))
      {
        PaintMapper(button, clickPt, 0, atom, true);
      }
    }

    void HandleXRayTool(u8 button, SPoint clickPt)
    {
      PaintMapper(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(),
                  Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), false, true);
    }

    void PaintMapper(u8 button, SPoint clickPt, s32 brushSize,
                     T atom, bool bucket, bool xray = false)
    {
      SPoint pt = GetAbsoluteLocation();
      pt.Set(clickPt.GetX() - pt.GetX(),
             clickPt.GetY() - pt.GetY());

      PaintAtom(*m_mainGrid, pt, brushSize, atom, bucket, xray);
    }

    void PaintAtom(Grid<GC>& grid, SPoint& clickPt, s32 brushSize,
                   T& atom, bool bucket, bool xray)
    {

      /* Only do this when tiles are together to keep from having to
       * deal with caches */
      if(!m_grend->IsRenderingTilesSeparated())
      {
        TileRenderer& tileRenderer = m_grend->GetTileRenderer();
        const SPoint& offset = tileRenderer.GetWindowTL();

        SPoint& cp = clickPt;

        /* Offset it by the corner */
        cp.SetX(cp.GetX() - offset.GetX());
        cp.SetY(cp.GetY() - offset.GetY());

        u32 atomSize = tileRenderer.GetAtomSize();

        /* Figure out which atom needs changing */
        cp.SetX(cp.GetX() / atomSize);
        cp.SetY(cp.GetY() / atomSize);

        if(brushSize > 0)
        {
          /* brushSize can't be templated, so let's do this by hand. */
          SPoint tile, site;
          for(s32 y = -brushSize; y <= brushSize; y++)
          {
            for(s32 x = -brushSize; x <= brushSize; x++)
            {
              SPoint pt(cp.GetX() + x, cp.GetY() + y);
              if(sqrt((x * x) + (y * y)) <= brushSize &&
                 grid.MapGridToTile(pt, tile, site))
              {
                if(xray)
                {
                  grid.XRayAtom(SPoint(cp.GetX() + x, cp.GetY() + y));
                }
                else
                {
                  grid.PlaceAtom(atom, SPoint(cp.GetX() + x, cp.GetY() + y));
                }
              }
            }
          }
        }
        else if(cp.GetX() >= 0 && cp.GetY() >= 0 &&
                cp.GetX() < TILE_SIDE_LIVE_SITES * W &&
                cp.GetY() < TILE_SIDE_LIVE_SITES * H)
        {
          if(bucket)
          {
            BucketFill(grid, atom, cp);
          }
          else
          {
            grid.PlaceAtom(atom, cp);
          }
        }
      }
    }

    void BucketFill(Grid<GC>& grid, T& atom, SPoint& pt)
    {
      MDist<1> md = MDist<1>::get();

      grid.PlaceAtom(atom, pt);

      for(u32 i = 0; i < md.GetTableSize(1); i++)
      {
        SPoint npt = md.GetPoint(i);
        npt.Add(pt.GetX(), pt.GetY());

        if(npt.GetX() >= 0 && npt.GetY() >= 0 &&
           npt.GetX() < TILE_SIDE_LIVE_SITES * W &&
           npt.GetY() < TILE_SIDE_LIVE_SITES * H)
        {
          if(Atom<CC>::IsType(*grid.GetAtom(npt), Element_Empty<CC>::THE_INSTANCE.GetType()))
          {
            BucketFill(grid, atom, npt);
          }
        }
      }
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;
      if(event.type == SDL_MOUSEBUTTONDOWN) {

        SPoint pt = GetAbsoluteLocation();
        pt.Set(event.x - pt.GetX(),
               event.y - pt.GetY());

        switch (event.button)
        {
        case SDL_BUTTON_LEFT:
          m_leftButtonDragStart = pt;
          m_leftButtonGridStart = m_grend->GetDrawOrigin();
          /* FALL THROUGH */
        case SDL_BUTTON_MIDDLE:
        case SDL_BUTTON_RIGHT:

          if(!mbe.m_keyboard.CtrlHeld() && m_paintingEnabled)
          {
            switch(mbe.m_selectedTool)
            {
            case TOOL_SELECTOR:
              HandleSelectorTool(mbe);
              break;
            case TOOL_ATOM_SELECTOR:
              HandleAtomSelectorTool(mbe);
              break;
            case TOOL_PENCIL:
              HandlePencilTool(mbe);
              break;
            case TOOL_ERASER:
              HandleEraserTool(mbe);
              break;
            case TOOL_BRUSH:
              HandleBrushTool(mbe);
              break;
            case TOOL_BUCKET:
              HandleBucketTool(mbe);
              break;
            case TOOL_XRAY:
              HandleXRayTool(mbe);
              break;

            default: break; /* Do the rest later */
            }
          }

          break;

        case SDL_BUTTON_WHEELUP:
          m_grend->IncreaseAtomSize(pt);
          break;

        case SDL_BUTTON_WHEELDOWN:
          m_grend->DecreaseAtomSize(pt);
          break;
        }
      }
      return true;
    }

    virtual bool Handle(MouseMotionEvent& mbe)
    {
      SDL_MouseMotionEvent & event = mbe.m_event.motion;
      if (mbe.m_keyboard.CtrlHeld())
      {
        if(mbe.m_buttonMask & (1 << SDL_BUTTON_LEFT))
        {
          SPoint nowAt(event.x, event.y);
          SPoint delta = nowAt - m_leftButtonDragStart;
          m_grend->SetDrawOrigin(m_leftButtonGridStart+delta);
        }
      }
      else
      {
        u8 mask = 0;
        if(mbe.m_buttonMask & (1 << SDL_BUTTON_LEFT))
        {
          mask = SDL_BUTTON_LEFT;
        }
        else if(mbe.m_buttonMask & (1 << SDL_BUTTON_RIGHT))
        {
          mask = SDL_BUTTON_RIGHT;
        }

        if(mask && !mbe.m_keyboard.CtrlHeld() && m_paintingEnabled)
        {
          switch(mbe.m_selectedTool)
          {
          case TOOL_PENCIL:
            HandlePencilTool(mask, SPoint(event.x, event.y));
            break;
          case TOOL_ERASER:
            HandleEraserTool(mask, SPoint(event.x, event.y));
            break;
          case TOOL_BRUSH:
            HandleBrushTool(mask, SPoint(event.x, event.y));
            break;
          case TOOL_XRAY:
            HandleXRayTool(mask, SPoint(event.x, event.y));
            break;
          case TOOL_ATOM_SELECTOR:
            HandleAtomSelectorTool(mask, SPoint(event.x, event.y));
            break;

          default:
            /* Some tools don't need to do this */
            break;
          }
        }
      }
      return false;
    }
  };
} /* namespace MFM */

#endif /* GRIDPANEL_H */
