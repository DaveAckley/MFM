#include "tilerenderer.h"
#include "eventwindow.h"

#define TILESIZE_CHANGE_RATE 1

TileRenderer::TileRenderer(Panel* panel)
{
  m_panel = panel;
  m_atomDrawSize = 8;
  m_drawMemRegions = true;
  m_drawGrid = true;
  m_drawDataHeat = false;
  m_gridColor = 0xff303030;
  m_sharedColor = 0xffe8757a;
  m_visibleColor = 0xfff0d470;
  m_hiddenColor = 0xff423b16;
  m_cacheColor = 0xffffc0c0;
  m_windowTL.SetX(0);
  m_windowTL.SetY(0);
}

void TileRenderer::RenderAtomBG(Point<int>& offset,
				Point<int>& atomLoc,
				u32 color)
{
  Drawing::FillRect(m_panel->GetDestination(),
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

void TileRenderer::ToggleDataHeat()
{
  m_drawDataHeat = !m_drawDataHeat;
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
