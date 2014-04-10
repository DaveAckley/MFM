#include "TileRenderer.h"
#include "EventWindow.h"

namespace MFM {

#define TILESIZE_CHANGE_RATE 1

TileRenderer::TileRenderer(SDL_Surface* dest)
{
  m_dest = dest;
  m_atomDrawSize = 8;
  m_drawMemRegions = FULL;
  m_drawGrid = true;
  m_drawDataHeat = false;
  m_gridColor = 0xff303030;
  m_sharedColor = 0xffe8757a;
  m_visibleColor = 0xfff0d470;
  m_hiddenColor = 0xff423b16;
  m_selectedHiddenColor = 0xffffffff;
  m_cacheColor = 0xffffc0c0;
  m_windowTL.SetX(0);
  m_windowTL.SetY(0);
}

void TileRenderer::RenderAtomBG(SPoint& offset,
				SPoint& atomLoc,
				u32 color)
{
  /*
   * Again, draw this rect manually in order to only draw
   * renderable pieces of it.
   *
   * (???  What? There's no clipping or anything I can see in here..
   * Why not use fillrect again?)
   */
  Point<s32> ulpt(m_windowTL.GetX() + offset.GetY() + atomLoc.GetX() *
		  m_atomDrawSize,
		  m_windowTL.GetY() + offset.GetY() + atomLoc.GetY() *
		  m_atomDrawSize);

  Point<s32> brpt(ulpt.GetX() + m_atomDrawSize, ulpt.GetY() + m_atomDrawSize);

  if(brpt.GetX() > (s32)m_dimensions.GetX())
  {
    brpt.SetX(m_dimensions.GetX());
  }
  if(brpt.GetY() > (s32)m_dimensions.GetY())
  {
    brpt.SetY(m_dimensions.GetY());
  }

  Drawing::FillRect(m_dest,
                    ulpt.GetX(),ulpt.GetY(),
                    brpt.GetX()-ulpt.GetX(),brpt.GetY()-ulpt.GetY(),
                    color);
}

void TileRenderer::ToggleGrid()
{
  m_drawGrid = !m_drawGrid;
}

void TileRenderer::ToggleMemDraw()
{
  m_drawMemRegions = (DrawRegionType) ((m_drawMemRegions+1)%MAX);
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
} /* namespace MFM */

