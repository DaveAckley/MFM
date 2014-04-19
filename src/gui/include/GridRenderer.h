#ifndef GRIDRENDERER_H      /* -*- C++ -*- */
#define GRIDRENDERER_H

#include "Grid.h"
#include "SDL/SDL.h"
#include "TileRenderer.h"

namespace MFM
{

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

    ~GridRenderer();

    void SetEventWindowRenderMode(EventWindowRenderMode mode);

    void SetDestination(SDL_Surface* dest);

    UPoint& GetDimensions();

    void SetDimensions(Point<u32> dimensions);

    void IncreaseAtomSize();

    void ToggleTileSeparation();

    void ToggleDataHeatmap();

    void DecreaseAtomSize();

    void ToggleGrid();

    void ToggleMemDraw();

    void MoveUp(u8 amount);

    void MoveDown(u8 amount);

    void MoveLeft(u8 amount);

    void MoveRight(u8 amount);

    void DeselectTile();

    SPoint& GetSelectedTile();

    template <class GC>
    void RenderGrid(Grid<GC>& grid);

    template <class GC>
    void SelectTile(Grid<GC>& grid, SPoint& clickPt);

  };
} /* namespace MFM */
#include "GridRenderer.tcc"

#endif /*GRIDRENDERER_H*/

