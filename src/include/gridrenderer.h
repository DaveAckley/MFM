#ifndef GRIDRENDERER_H      /* -*- C++ -*- */
#define GRIDRENDERER_H

#include "grid.h"
#include "SDL/SDL.h"
#include "tilerenderer.h"

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

  static const EventWindowRenderMode m_defaultRenderMode =
    EVENTWINDOW_RENDER_ALL;

  static const bool m_renderTilesSeparatedDefault = false;

  bool m_renderTilesSeparated;

  EventWindowRenderMode m_currentEWRenderMode;

  template <class T,u32 R>
  void RenderGridClose(Grid<T,R>& grid);

  template <class T,u32 R>
  void RenderGridSeparated(Grid<T,R>& grid);

public:
  GridRenderer(SDL_Surface* dest);

  GridRenderer(SDL_Surface* dest,
	       TileRenderer* tr);

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

  void IncreaseAtomSize()
  {
    m_tileRenderer->IncreaseAtomSize();
  }

  void ToggleTileSeparation()
  {
    m_renderTilesSeparated = !m_renderTilesSeparated;
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
  
  template <class T,u32 R>
  void RenderGrid(Grid<T,R>& grid);
  
};

#include "gridrenderer.tcc"

#endif /*GRIDRENDERER_H*/
