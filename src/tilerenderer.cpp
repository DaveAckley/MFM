#include "drawing.h"
#include "tilerenderer.h"
#include "eventwindow.h"

TileRenderer::TileRenderer(SDL_Surface* dest)
{
  m_dest = dest;
  m_atomDrawSize = 8;
  m_drawMemRegions = true;
  m_drawGrid = true;
  m_gridColor = 0xff303030;
  m_sharedColor = 0xffe8757a;
  m_visibleColor = 0xfff0d470;
  m_hiddenColor = 0xff423b16;
  m_windowTL.SetX(0);
  m_windowTL.SetY(0);
}

void TileRenderer::RenderMemRegions(Point<int>* pt)
{
  RenderMemRegion(pt, 0, m_sharedColor);
  RenderMemRegion(pt, 1, m_visibleColor);
  RenderMemRegion(pt, 2, m_hiddenColor);
}

void TileRenderer::RenderMemRegion(Point<int>* pt,
				   int regID,
				   Uint32 color)
{

  int tileSize = m_atomDrawSize * TILE_WIDTH;
  int ewrSize = EVENT_WINDOW_RADIUS * m_atomDrawSize;

  Drawing::FillRect(m_dest,
		    pt->GetX() + (ewrSize * regID),
		    pt->GetY() + (ewrSize * regID),
		    tileSize - (ewrSize * regID * 2),
		    tileSize - (ewrSize * regID * 2),
		    color);
}

void TileRenderer::RenderGrid(Point<int>* pt)
{
  int lineLen = m_atomDrawSize * TILE_WIDTH;
  for(int x = 0; x < TILE_WIDTH + 1; x++)
  {
    Drawing::DrawVLine(m_dest, x * m_atomDrawSize,
		       pt->GetY(), 
		       pt->GetY() + lineLen,
		       m_gridColor);
  }
  for(int y = 0; y < TILE_WIDTH + 1; y++)
  {
    Drawing::DrawHLine(m_dest, y * m_atomDrawSize,
		       pt->GetX(),
		       pt->GetX() + lineLen,
		       m_gridColor);
  }
}

void TileRenderer::RenderTile(Tile* t, Point<int>* pt)
{
  if(m_drawMemRegions)
  {
    RenderMemRegions(pt);
  }
  if(m_drawGrid)
  {
    RenderGrid(pt);
  }
}

void TileRenderer::ToggleGrid()
{
  m_drawGrid = !m_drawGrid;
}

void TileRenderer::ToggleMemDraw()
{
  m_drawMemRegions = !m_drawMemRegions;
}
