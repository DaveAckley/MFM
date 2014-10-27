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
    enum { MAX_BUCKET_FILL_DEPTH = 10000 };

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

    /* I'm making this a field because of the recursive nature of BucketFill .*/
    u32 m_bucketFillStartType;

    SPoint m_cloneOrigin;
    SPoint m_cloneDestination;

    AtomViewPanel<GC> m_atomViewPanel;

   public:
    GridPanel() :
      m_paintingEnabled(false),
      m_bucketFillStartType(0),
      m_cloneOrigin(-1, -1),
      m_cloneDestination(-1, -1)
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
      m_atomViewPanel.SetVisibility(false);

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

    void ToggleAtomViewPanel()
    {
      m_atomViewPanel.ToggleVisibility();
    }

    void ToggleDrawAtomsAsSquares()
    {
      if(m_grend)
      {
        m_grend->ToggleDrawAtomsAsSquares();
      }
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
      m_cloneOrigin.Set(-1, -1);
      m_grend->SetCloneOrigin(m_cloneOrigin);
      m_atomViewPanel.SetAtom(NULL);
    }

   protected:
    virtual void PaintComponent(Drawing& drawing)
    {
      this->Panel::PaintComponent(drawing);

      m_grend->RenderGrid(drawing, *m_mainGrid, m_toolboxPanel->GetBrushSize());
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

    void HandleAirbrushTool(MouseButtonEvent& mbe)
    {
      HandleAirbrushTool(mbe.m_event.button.button,
                         SPoint(mbe.m_event.button.x,
                                mbe.m_event.button.y));
    }

    SPoint ClickPointToAtom(const SPoint& clickPt)
    {
      SPoint abs = GetAbsoluteLocation();
      abs.Set(clickPt.GetX() - abs.GetX(),
              clickPt.GetY() - abs.GetY());

      TileRenderer& tileRenderer = m_grend->GetTileRenderer();
      const SPoint& offset = tileRenderer.GetWindowTL();


      abs.Set(abs.GetX() - offset.GetX(),
              abs.GetY() - offset.GetY());

      u32 atomSize = tileRenderer.GetAtomSize();

      abs.Set(abs.GetX() / atomSize,
              abs.GetY() / atomSize);

      return abs;
    }

    void HandleCloneTool(MouseButtonEvent& mbe)
    {
      if(mbe.m_event.button.button == SDL_BUTTON_LEFT)
      {
        m_cloneDestination = ClickPointToAtom(SPoint(mbe.m_event.button.x,
                                                     mbe.m_event.button.y));
      }
      HandleCloneTool(mbe.m_event.button.button,
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
      m_atomViewPanel.SetAtom(m_mainGrid->GetWritableAtom(selectedAtom));
    }

    void HandlePencilTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintAtom(button, clickPt, 0, atom, TOOL_PENCIL);
    }

    void HandleBrushTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintAtom(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(), atom, TOOL_BRUSH);
    }

    void HandleAirbrushTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintAtom(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(), atom, TOOL_AIRBRUSH);
    }

    void HandleEraserTool(u8 button, SPoint clickPt)
    {
      PaintAtom(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(),
                  Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), TOOL_ERASER);
    }

    void HandleBucketTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
        m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
        m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

        PaintAtom(button, clickPt, 0, atom, TOOL_BUCKET);
    }

    void HandleXRayTool(u8 button, SPoint clickPt)
    {
      PaintAtom(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(),
                  Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), TOOL_XRAY);
    }

    void HandleCloneTool(u8 button, SPoint clickPt)
    {
      PaintAtom(button, clickPt, (s32)m_toolboxPanel->GetBrushSize(),
                  Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), TOOL_CLONE);
    }

    void PaintAtom(u8 button, SPoint& clickPt, s32 brushSize,
                   const T& atom, EditingTool tool)
    {
      /* Only do this when tiles are together to keep from having to
       * deal with caches */
      if(!m_grend->IsRenderingTilesSeparated())
      {
        Grid<GC>& grid = *m_mainGrid;
        SPoint cp = ClickPointToAtom(clickPt);
        if(brushSize > 0)
        {
          /* brushSize can't be templated, so let's do this by hand. */
          SPoint tile, site;
          brushSize--;
          const s32 brushSqr = brushSize * brushSize;
          s32 ysqr;
          for(s32 y = -brushSize; y <= brushSize; y++)
          {
            ysqr = y * y;
            for(s32 x = -brushSize; x <= brushSize; x++)
            {
              SPoint pt(cp.GetX() + x, cp.GetY() + y);
              if(((x * x) + ysqr) <= brushSqr &&
                 grid.MapGridToTile(pt, tile, site))
              {
                if(tool == TOOL_XRAY)
                {
                  grid.MaybeXRayAtom(pt);
                }
                else if(tool == TOOL_CLONE)
                {
                  if(button == SDL_BUTTON_RIGHT)
                  {
                    SPoint tile, site;
                    if(grid.MapGridToTile(cp, tile, site))
                    {
                      m_cloneOrigin.Set(cp.GetX(), cp.GetY());
                      m_grend->SetCloneOrigin(m_cloneOrigin);
                    }
                    return; /* Only need to do this once. */
                  }
                  else
                  {
                    if(m_cloneOrigin.GetX() >= 0 && m_cloneOrigin.GetY() >= 0)
                    {
                      SPoint clonePt(m_cloneOrigin.GetX() +
                                     (pt.GetX() - m_cloneDestination.GetX()),
                                     m_cloneOrigin.GetY() +
                                     (pt.GetY() - m_cloneDestination.GetY()));
                      SPoint tile, site;
                      if(grid.MapGridToTile(clonePt, tile, site))
                      {
                        const T* a =  grid.GetAtom(clonePt);
                        grid.PlaceAtom(*a, pt);
                      }
                    }
                  }
                }
                else if((tool != TOOL_AIRBRUSH) ||
                        (m_mainGrid->GetRandom().OneIn(50)))
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
          if(tool == TOOL_BUCKET)
          {
            SPoint tile, site;
            if(grid.MapGridToTile(cp, tile, site))
            {
              m_bucketFillStartType = grid.GetTile(tile).GetAtom(site)->GetType();
              if(m_bucketFillStartType != atom.GetType())
              {
                BucketFill(grid, atom, cp, MAX_BUCKET_FILL_DEPTH);
              }
            }
          }
          else if(tool == TOOL_CLONE)
          {
            if(button == SDL_BUTTON_RIGHT)
            {
              SPoint tile, site;
              if(grid.MapGridToTile(cp, tile, site))
              {
                m_cloneOrigin.Set(cp.GetX(), cp.GetY());
                LOG.Debug("Clone Origin Set: (%d, %d)", cp.GetX(), cp.GetY());
              }
            }
            else
            {
              SPoint clonePt(m_cloneOrigin.GetX() +
                             (cp.GetX() - m_cloneDestination.GetX()),
                             m_cloneOrigin.GetY() +
                             (cp.GetY() - m_cloneDestination.GetY()));
              SPoint tile, site;
              LOG.Debug("Cloning from (%d, %d) to (%d, %d) [clone dest = (%d, %d)]",
                        clonePt.GetX(), clonePt.GetY(),
                        cp.GetX(), cp.GetY(),
                        m_cloneDestination.GetX(), m_cloneDestination.GetY());
              if(grid.MapGridToTile(clonePt, tile, site))
              {
                const T* a =  grid.GetAtom(clonePt);
                grid.PlaceAtom(*a, cp);
              }
            }
          }
          else
          {
            grid.PlaceAtom(atom, cp);
          }
        }
      }
    }

    void BucketFill(Grid<GC>& grid, const T& atom, SPoint& pt, u32 depth)
    {
      grid.PlaceAtom(atom, pt);
      SPoint npt;
      for(u32 i = MDist<1>::get().GetFirstIndex(1); i <= MDist<1>::get().GetLastIndex(1); i++)
      {
        npt = MDist<1>::get().GetPoint(i);
        npt.Add(pt.GetX(), pt.GetY());

        if(npt.GetX() >= 0 && npt.GetY() >= 0 &&
           npt.GetX() < TILE_SIDE_LIVE_SITES * W &&
           npt.GetY() < TILE_SIDE_LIVE_SITES * H)
        {
          if(Atom<CC>::IsType(*grid.GetAtom(npt),
                              m_bucketFillStartType))
          {
            if(depth)
            {
              BucketFill(grid, atom, npt, depth - 1);
            }
          }
        }
      }
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;
      if(event.type == SDL_MOUSEBUTTONDOWN)
      {
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
            case TOOL_CLONE:
              HandleCloneTool(mbe);
              break;
            case TOOL_AIRBRUSH:
              HandleAirbrushTool(mbe);
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

        m_grend->SetHoveredAtom(*m_mainGrid, SPoint(event.x, event.y));

        if(!mask)
        {
          m_grend->SetHoveredAtom(*m_mainGrid, SPoint(event.x, event.y));
        }
        else if(mask && !mbe.m_keyboard.CtrlHeld() && m_paintingEnabled)
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
          case TOOL_CLONE:
            HandleCloneTool(mask, SPoint(event.x, event.y));
            break;
          case TOOL_AIRBRUSH:
            HandleAirbrushTool(mask, SPoint(event.x, event.y));
            break;
          default:
            /* Some tools don't need to do this */
            break;
          }
        }
        else
        {
          m_grend->DeselectHoveredAtom();
        }
      }
      return false;
    }
  };
} /* namespace MFM */

#endif /* GRIDPANEL_H */
