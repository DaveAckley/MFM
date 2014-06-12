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
#include "Panel.h"
#include "GridRenderer.h"
#include "EditingTool.h"

#define SCREEN_INITIAL_WIDTH 1280
#define SCREEN_INITIAL_HEIGHT 1024

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
      /* TODO Maybe be able to select two types of atoms? Like in most
       * image editors, right click allows painting of a different
       * color.*/
      if(mbe.m_event.button.button == SDL_BUTTON_LEFT)
      {
	SPoint pt = GetAbsoluteLocation();
	pt.Set(mbe.m_event.button.x - pt.GetX(),
	       mbe.m_event.button.y - pt.GetY());

	T atom = Element_Wall<CC>::THE_INSTANCE.GetDefaultAtom();

	PaintAtom(*m_mainGrid, pt, 1, atom);
      }
    }

    void PaintAtom(Grid<GC>& grid, SPoint& clickPt, u32 radius, T& atom)
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
	u32 tileSize = (GC::CORE_CONFIG::PARAM_CONFIG::TILE_WIDTH -
			2 * GC::CORE_CONFIG::PARAM_CONFIG::EVENT_WINDOW_RADIUS);

	/* Figure out which atom needs changing */
	cp.SetX(cp.GetX() / atomSize);
	cp.SetY(cp.GetY() / atomSize);

	SPoint tilePt(cp.GetX() / tileSize, cp.GetY() / tileSize);

	grid.PlaceAtom(atom, SPoint(cp.GetX(), cp.GetY()));
      }

    }

    void HandleBucketTool(MouseButtonEvent& mbe)
    {

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
	else
	{
	  /* Paint atoms */
	}
      }
      return false;
    }

  };

} /* namespace MFM */

#endif /* GRIDPANEL_H */
