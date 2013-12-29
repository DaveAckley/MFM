#include "tilerenderer.h"
#include "eventwindow.h"

#define TILESIZE_CHANGE_RATE 1

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

void TileRenderer::RenderMemRegions(Point<int>& pt)
{
  RenderMemRegion(pt, 0, m_sharedColor);
  RenderMemRegion(pt, 1, m_visibleColor);
  RenderMemRegion(pt, 2, m_hiddenColor);
}

void TileRenderer::RenderMemRegion(Point<int>& pt,
				      int regID,
				      Uint32 color)
{

  /* Subtract out the cache's width */
  int tileSize = m_atomDrawSize * 
    (TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS);
  int ewrSize = EVENT_WINDOW_RADIUS * m_atomDrawSize;

  Drawing::FillRect(m_dest,
		    pt.GetX() + (ewrSize * regID) +
		    m_windowTL.GetX(),
		    pt.GetY() + (ewrSize * regID) +
		    m_windowTL.GetY(),
		    tileSize - (ewrSize * regID * 2),
		    tileSize - (ewrSize * regID * 2),
		    color);
}

void TileRenderer::RenderGrid(Point<int>* pt)
{
  int lineLen = m_atomDrawSize * 
    (TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS);
  int linesToDraw = TILE_WIDTH + 
    1 - (2 * EVENT_WINDOW_RADIUS);
  for(int x = 0; x < linesToDraw; x++)
  {
    Drawing::DrawVLine(m_dest,
		       pt->GetX() + x * m_atomDrawSize +
		       m_windowTL.GetX(),
		       pt->GetY() + m_windowTL.GetY(),
		       pt->GetY() + lineLen +
		       m_windowTL.GetY(),
		       m_gridColor);
  }

  for(int y = 0; y < linesToDraw; y++)
  {
    Drawing::DrawHLine(m_dest,
		       pt->GetY() + y * m_atomDrawSize +
		       m_windowTL.GetY(),
		       pt->GetX() + m_windowTL.GetX(),
		       pt->GetX() + lineLen +
		       m_windowTL.GetX(),
		       m_gridColor);
  }
}

void TileRenderer::RenderAtomBG(Point<int>& offset,
				Point<int>& atomLoc,
				u32 color)
{
  Drawing::FillRect(m_dest,
		    m_windowTL.GetX() +
		    offset.GetX() + atomLoc.GetX() * m_atomDrawSize,
		    m_windowTL.GetY() + 
		    offset.GetY() + atomLoc.GetY() * m_atomDrawSize,
		    m_atomDrawSize, m_atomDrawSize, color);
}

void TileRenderer::ToggleGrid()
{
  m_drawGrid = !m_drawGrid;
}

void TileRenderer::ToggleMemDraw()
{
  m_drawMemRegions = !m_drawMemRegions;
}

void TileRenderer::IncreaseAtomSize()
{
  m_atomDrawSize += TILESIZE_CHANGE_RATE;
}
  
void TileRenderer::DecreaseAtomSize()
{
  if(m_atomDrawSize > 1)
  {
    m_atomDrawSize -= TILESIZE_CHANGE_RATE;
  }
}

void TileRenderer::MoveUp(u8 amount)
{
  m_windowTL.SetY(m_windowTL.GetY() + amount);
}

void TileRenderer::MoveDown(u8 amount)
{
  m_windowTL.SetY(m_windowTL.GetY() - amount);
}

void TileRenderer::MoveLeft(u8 amount)
{
  m_windowTL.SetX(m_windowTL.GetX() + amount);
}

void TileRenderer::MoveRight(u8 amount)
{
  m_windowTL.SetX(m_windowTL.GetX() - amount);
}
