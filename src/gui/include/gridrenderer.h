#ifndef GRIDRENDERER_H      /* -*- C++ -*- */
#define GRIDRENDERER_H

#include "grid.h"
#include "SDL/SDL.h"
#include "tilerenderer.h"

namespace MFM {

typedef enum
{
  EVENTWINDOW_RENDER_OFF     = 0,
  EVENTWINDOW_RENDER_CURRENT = 1,
  EVENTWINDOW_RENDER_ALL     = 2
} EventWindowRenderMode;


class GridRenderer
{
private:

  SDL_Surface* m_dest;

  TileRenderer* m_tileRenderer;

  Point<u32> m_dimensions;

  SPoint m_selectedTile;

  static const EventWindowRenderMode m_defaultRenderMode =
    EVENTWINDOW_RENDER_OFF;

  static const bool m_renderTilesSeparatedDefault = false;

  bool m_renderTilesSeparated;

  EventWindowRenderMode m_currentEWRenderMode;

public:
  GridRenderer(SDL_Surface* dest);

  GridRenderer(TileRenderer* tr);

  GridRenderer();

  void SetEventWindowRenderMode(EventWindowRenderMode mode)
  {
    m_currentEWRenderMode = mode;
  }

  void SetDestination(SDL_Surface* dest)
  {
    m_dest = dest;
    delete m_tileRenderer;
    m_tileRenderer = new TileRenderer(dest);
  }

  Point<u32>& GetDimensions()
  {
    return m_dimensions;
  }

  void SetDimensions(Point<u32> dimensions)
  {
    m_dimensions = dimensions;
    m_tileRenderer->SetDimensions(dimensions);
  }

  void IncreaseAtomSize()
  {
    m_tileRenderer->IncreaseAtomSize();
  }

  void ToggleTileSeparation()
  {
    m_renderTilesSeparated = !m_renderTilesSeparated;
  }

  void ToggleDataHeatmap()
  {
    m_tileRenderer->ToggleDataHeat();
  }

  void DecreaseAtomSize()
  {
    m_tileRenderer->DecreaseAtomSize();
  }

  void ToggleGrid()
  {
    m_tileRenderer->ToggleGrid();
  }

  void ToggleMemDraw()
  {
    m_tileRenderer->ToggleMemDraw();
  }

  void MoveUp(u8 amount)
  {
    m_tileRenderer->MoveUp(amount);
  }

  void MoveDown(u8 amount)
  {
    m_tileRenderer->MoveDown(amount);
  }

  void MoveLeft(u8 amount)
  {
    m_tileRenderer->MoveLeft(amount);
  }

  void MoveRight(u8 amount)
  {
    m_tileRenderer->MoveRight(amount);
  }

  ~GridRenderer();

  template <class GC>
  void RenderGrid(Grid<GC>& grid);

  template <class GC>
  SPoint& GetSelectedTile()
  {
    return m_selectedTile;
  }

  template <class GC>
  void SelectTile(Grid<GC>& grid, SPoint& clickPt)
  {
    SPoint& offset = m_tileRenderer->GetWindowTL();

    SPoint& cp = clickPt;

    /* Offset it by the corner */
    cp.SetX(cp.GetX() - offset.GetX());
    cp.SetY(cp.GetY() - offset.GetY());

    u32 tileSize = m_tileRenderer->GetAtomSize() * (GC::CORE_CONFIG::PARAM_CONFIG::TILE_WIDTH + 1);
    if(!m_renderTilesSeparated)
    {
      tileSize -= m_tileRenderer->GetAtomSize() * (GC::CORE_CONFIG::PARAM_CONFIG::EVENT_WINDOW_RADIUS * 2 + 1);
    }

    m_selectedTile.Set(-1, -1);

    if(cp.GetX() > 0 && cp.GetY() > 0)
    {
      for(u32 x = 0; x < GC::GRID_WIDTH + 1; x++)
      {
	if(x * tileSize >= (u32)cp.GetX())
	{
	  m_selectedTile.SetX(x - 1);
	  break;
	}
      }
      for(u32 y = 0; y < GC::GRID_HEIGHT + 1; y++)
      {
	if(y * tileSize >= (u32)cp.GetY())
	{
	  m_selectedTile.SetY(y - 1);
	  break;
	}
      }
    }
  }

};
} /* namespace MFM */
#include "gridrenderer.tcc"

#endif /*GRIDRENDERER_H*/

