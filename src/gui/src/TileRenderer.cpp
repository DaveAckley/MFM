#include "TileRenderer.h"
#include "EventWindow.h"

namespace MFM
{

  void TileRenderer::TileRendererSaveDetails(ByteSink & sink) const
  {
    sink.Printf(" PP(traz=%d)\n", m_atomDrawSize);
    sink.Printf(" PP(trbt=%d)\n", m_drawBackgroundType);
    sink.Printf(" PP(trdg=%d)\n", m_drawGrid);
    sink.Printf(" PP(trdh=%d)\n", m_drawDataHeat);
    sink.Printf(" PP(trft=%d)\n", m_drawForegroundType);
    sink.Printf(" PP(trrs=%d)\n", m_renderSquares);

    {
      SPoint tmp(m_windowTL);
      SPointSerializer sp(tmp);
      sink.Printf(" PP(trtl=%@)\n",&sp);
    }
    {
      UPoint tmp(m_dimensions);
      UPointSerializer up(tmp);
      sink.Printf(" PP(trdm=%@)\n",&up);
    }
  }

  bool TileRenderer::TileRendererLoadDetails(const char * key, LineCountingByteSource & source)
  {
    if (!strcmp("traz",key)) return 1 == source.Scanf("%?d", sizeof m_atomDrawSize, &m_atomDrawSize);
    if (!strcmp("trbt",key)) return 1 == source.Scanf("%?d", sizeof m_drawBackgroundType, &m_drawBackgroundType);
    if (!strcmp("trdg",key)) return 1 == source.Scanf("%?d", sizeof m_drawGrid, &m_drawGrid);
    if (!strcmp("trdh",key)) return 1 == source.Scanf("%?d", sizeof m_drawDataHeat, &m_drawDataHeat);
    if (!strcmp("trft",key)) return 1 == source.Scanf("%?d", sizeof m_drawForegroundType, &m_drawForegroundType);
    if (!strcmp("trrs",key)) return 1 == source.Scanf("%?d", sizeof m_renderSquares, &m_renderSquares);

    if (!strcmp("trtl",key))
    {
      SPointSerializer sp(m_windowTL);
      return 1 == source.Scanf("%@",&sp);
    }

    if (!strcmp("trdm",key))
    {
      UPointSerializer up(m_dimensions);
      return 1 == source.Scanf("%@",&up);
    }

    return false;
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

    m_hiddenColor  = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 76);
    m_visibleColor = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 84);
    m_sharedColor  = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 92);
    m_cacheColor   = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 100);

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
