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

#include "itype.h"
#include "MDist.h"
#include "Panel.h"
#include "GridRenderer.h"
#include "EditingTool.h"
#include "ToolboxPanel.h"

#define SCREEN_INITIAL_WIDTH 1280
#define SCREEN_INITIAL_HEIGHT 1024
#define BRUSH_SIZE 4

namespace MFM {

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

  public:
    GridPanel()
    {
      SetName("Grid Panel");
      SetDimensions(SCREEN_INITIAL_WIDTH,
                    SCREEN_INITIAL_HEIGHT);
      SetRenderPoint(SPoint(0, 0));
      SetForeground(Drawing::BLACK);
      SetBackground(Drawing::BLACK);

      m_grend = NULL;
      m_mainGrid = NULL;
    }

    void SetGrid(OurGrid* mainGrid)
    {
      m_mainGrid = mainGrid;
    }

    void SetGridRenderer(GridRenderer* grend)
    {
      m_grend = grend;
    }

    void SetToolboxPanel(ToolboxPanel<CC>* toolboxPanel)
    {
      m_toolboxPanel = toolboxPanel;
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

    void HandlePencilTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
		m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
		m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintMapper(button, clickPt, false, atom, false);
    }

    void HandleBrushTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
		m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
		m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      PaintMapper(button, clickPt, true, atom, false);
    }

    void HandleEraserTool(u8 button, SPoint clickPt)
    {
      PaintMapper(button, clickPt, false,
		  Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), false);
    }

    void HandleBucketTool(u8 button, SPoint clickPt)
    {
      T atom = (button == SDL_BUTTON_LEFT) ?
		m_toolboxPanel->GetPrimaryElement()->GetDefaultAtom() :
		m_toolboxPanel->GetSecondaryElement()->GetDefaultAtom();

      /* Filling empty results in stack overflow for obvious reasons */
      if(!Atom<CC>::IsType(atom, Element_Empty<CC>::TYPE()))
      {
	PaintMapper(button, clickPt, false, atom, true);
      }
    }

    void PaintMapper(u8 button, SPoint clickPt, bool brush, T atom, bool bucket)
    {
      SPoint pt = GetAbsoluteLocation();
      pt.Set(clickPt.GetX() - pt.GetX(),
	     clickPt.GetY() - pt.GetY());

      PaintAtom(*m_mainGrid, pt, brush, atom, bucket);
    }

    void PaintAtom(Grid<GC>& grid, SPoint& clickPt, bool brush, T& atom, bool bucket)
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

	if(brush)
	{
	  MDist<BRUSH_SIZE> md = MDist<BRUSH_SIZE>::get();

	  for(u32 i = 0; i < md.GetTableSize(BRUSH_SIZE); i++)
	  {
	    SPoint pt = md.GetPoint(i);
	    pt.Add(cp.GetX(), cp.GetY());

	    if(pt.GetX() >= 0 && pt.GetY() >= 0 &&
		pt.GetX() < TILE_SIDE_LIVE_SITES * W &&
		pt.GetY() < TILE_SIDE_LIVE_SITES * H)
	    {
	      grid.PlaceAtom(atom, pt);
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
	  if(Atom<CC>::IsType(*grid.GetAtom(npt), Element_Empty<CC>::TYPE()))
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

        switch (event.button) {

	case SDL_BUTTON_LEFT:
          m_leftButtonDragStart = pt;
          m_leftButtonGridStart = m_grend->GetDrawOrigin();
	  /* FALL THROUGH */
	case SDL_BUTTON_MIDDLE:
	case SDL_BUTTON_RIGHT:

	  switch(mbe.m_selectedTool)
	  {
	  case TOOL_SELECTOR:
	    HandleSelectorTool(mbe);
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

	  default: break; /* Do the rest later */
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

	if(mask)
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
