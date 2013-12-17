#include "drawing.h"
#include "tilerenderer.h"

TileRenderer::TileRenderer(SDL_Surface* dest)
{
  m_dest = dest;
  m_atomDrawSize = 8;
  m_drawGrid = false;
  m_gridColor = 0xff303030;
  m_windowTL.SetX(0);
  m_windowTL.SetY(0);
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
  RenderGrid(pt);
}

void TileRenderer::ToggleGrid()
{
  m_drawGrid = !m_drawGrid;
}
