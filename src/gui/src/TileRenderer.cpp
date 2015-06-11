#include "TileRenderer.h"
#include "EventWindow.h"

namespace MFM
{

  void TileRenderer::SaveDetails(ByteSink & sink) const
  {
    sink.Printf(",%D%D%D%D%D%D",
                m_drawGrid,
                m_drawBackgroundType,
                m_drawDataHeat,
                m_atomDrawSize,
                m_renderSquares,
                m_drawForegroundType);

    {
      SPoint tmp(m_windowTL);
      SPointSerializer sp(tmp);
      sink.Printf(",%@",&sp);
    }
    {
      UPoint tmp(m_dimensions);
      UPointSerializer up(tmp);
      sink.Printf(",%@",&up);
    }
  }

  bool TileRenderer::LoadDetails(LineCountingByteSource & source)
  {
    u32 tmp[6];
    if (7 != source.Scanf(",%D%D%D%D%D%D",
                          &tmp[0], &tmp[1], &tmp[2],
                          &tmp[3], &tmp[4], &tmp[5]))
      return false;

    SPoint tmps(m_windowTL);
    SPointSerializer sp(tmps);
    if (2 != source.Scanf(",%@",&sp))
      return false;

    UPoint tmpu(m_dimensions);
    UPointSerializer up(tmpu);
    if (2 != source.Scanf(",%@",&up))
      return false;

    m_drawGrid = tmp[0];
    m_drawBackgroundType = (DrawBackgroundType) tmp[1];
    m_drawDataHeat = tmp[2];
    m_atomDrawSize = tmp[3];
    m_renderSquares = tmp[4];
    m_drawForegroundType = (DrawForegroundType) tmp[5];

    m_windowTL = tmps;
    m_dimensions = tmpu;

    return true;
  }


#define MAX_ATOM_SIZE 256

  TileRenderer::TileRenderer()
  {
    m_atomDrawSize = 8;
    m_drawBackgroundType = DRAW_BACKGROUND_DARK_TILE;
    m_drawGrid = true;
    m_drawDataHeat = false;
    m_renderSquares = false;
    m_gridColor = 0xff202020;
    m_drawForegroundType = DRAW_FOREGROUND_ELEMENT;

#if 0 // Too much range for me..  Also we'd like a lighter palette background on some choice..
    m_hiddenColor  = 0xff353535;
    m_visibleColor = 0xff595959;
    m_sharedColor  = 0xff959595;
    m_cacheColor   = 0xffc0c0c0;
#else
    m_hiddenColor  = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 76);
    m_visibleColor = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 84);
    m_sharedColor  = Drawing::InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 92);
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

  u32 TileRenderer::NextDrawBackgroundType()
  {
    return
      m_drawBackgroundType =
      (DrawBackgroundType) ((m_drawBackgroundType + 1) % DRAW_BACKGROUND_TYPE_COUNT);
  }

  const char * TileRenderer::GetDrawBackgroundTypeName() const
  {
    switch (m_drawBackgroundType)
    {
    case DRAW_BACKGROUND_LIGHT_TILE:  return "Light tile";
    case DRAW_BACKGROUND_NONE:        return "None";
    case DRAW_BACKGROUND_DARK_TILE:   return "Dark tile";
    case DRAW_BACKGROUND_CHANGE_AGE:  return "Change age";
    case DRAW_BACKGROUND_SITE:        return "Site";
    default:
      return "unknown";
    }
  }

  const char * TileRenderer::GetDrawForegroundTypeName() const
  {
    switch (m_drawForegroundType)
    {
    case DRAW_FOREGROUND_ELEMENT:  return "Element";
    case DRAW_FOREGROUND_ATOM_1:   return "Atom #1";
    case DRAW_FOREGROUND_ATOM_2:   return "Atom #2";
    case DRAW_FOREGROUND_ATOM_3:   return "Atom #3";
    case DRAW_FOREGROUND_SITE:     return "Site";
    case DRAW_FOREGROUND_NONE:     return "None";
    default:
      return "unknown";
    }
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

  void TileRenderer::Move(SPoint amount)
  {
    m_windowTL += amount;
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
