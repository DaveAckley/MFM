#include "TileRenderer.h"
#include "EventWindow.h"

namespace MFM
{

#define MAX_ATOM_SIZE 256

  TileRenderer::TileRenderer()
  {
    m_atomDrawSize = 8;
    m_drawMemRegions = NO;
    m_drawGrid = true;
    m_drawDataHeat = false;
    m_renderSquares = false;
    m_gridColor = 0xff202020;

#if 0 // Too much range for me..  Also we'd like a lighter palette background on some choice..
    m_hiddenColor  = 0xff353535;
    m_visibleColor = 0xff595959;
    m_sharedColor  = 0xff959595;
    m_cacheColor   = 0xffc0c0c0;
#else
    m_hiddenColor  = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 85);
    m_visibleColor = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 90);
    m_sharedColor  = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 95);
    m_cacheColor   = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 100);
#endif
    m_selectedHiddenColor = 0xffffffff;
    m_selectedPausedColor = 0xffafafaf;
    m_windowTL.SetX(0);
    m_windowTL.SetY(0);
  }

  void TileRenderer::RenderAtomBG(Drawing & drawing,
                                  SPoint& offset,
                                  SPoint& atomLoc,
                                  u32 color)
  {
    SPoint ulpt(m_windowTL.GetX() + offset.GetY() + atomLoc.GetX() *
                m_atomDrawSize,
                m_windowTL.GetY() + offset.GetY() + atomLoc.GetY() *
                m_atomDrawSize);

    SPoint brpt(ulpt.GetX() + m_atomDrawSize, ulpt.GetY() + m_atomDrawSize);

    if(brpt.GetX() > (s32)m_dimensions.GetX())
    {
      brpt.SetX(m_dimensions.GetX());
    }
    if(brpt.GetY() > (s32)m_dimensions.GetY())
    {
      brpt.SetY(m_dimensions.GetY());
    }

    drawing.FillRect(ulpt.GetX(),ulpt.GetY(),
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

  void TileRenderer::ChangeAtomSize(bool increase, SPoint around)
  {
    SPoint atomLoc = (around - m_windowTL) / m_atomDrawSize;

    const u32 SCALE_GRANULARITY = 10;
    s32 amount;

    if (m_atomDrawSize < SCALE_GRANULARITY)   // Be proportional unless tiny
    {
      amount = 1;
    }
    else
    {
      amount = m_atomDrawSize / SCALE_GRANULARITY;
    }

    if (!increase)
    {
      amount = -amount;
    }


    s32 newSize = m_atomDrawSize + amount;

    if (newSize < 1)
    {
      newSize = 1;
    }
    else if (newSize > MAX_ATOM_SIZE)
    {
      newSize = MAX_ATOM_SIZE;
    }

    m_atomDrawSize = newSize;

    SPoint newAround = atomLoc * m_atomDrawSize + m_windowTL;
    SPoint delta = newAround - around;
    m_windowTL -= delta;
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
