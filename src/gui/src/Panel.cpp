#include "Panel.h"
#include "Drawing.h"

namespace MFM {

  Panel::Panel()
  {
    m_dest = NULL;
    m_renderPt = Point<s32>(0, 0);
  }

  Panel::Panel(u32 width, u32 height)
  {
    m_width = width;
    m_height = height;
    m_renderPt = Point<s32>(0, 0);
  }

  Panel::Panel(SDL_Surface* dest)
  {
    m_dest = dest;
    m_renderPt = Point<s32>(0, 0);
  }

  Panel::Panel(SDL_Surface* dest, u32 width, u32 height)
  {
    m_dest = dest;
    m_width = width;
    m_height = height;
    m_renderPt = Point<s32>(0, 0);
  }

  void Panel::SetDestination(SDL_Surface* dest)
  {
    m_dest = dest;
  }

  void Panel::SetDimensions(u32 width, u32 height)
  {
    m_width = width;
    m_height = height;
  }

  void Panel::SetRenderPoint(Point<s32>& renderPt)
  {
    m_renderPt = renderPt;
  }

  void Panel::Render(void* rendObj)
  {
    Drawing::DrawHLine(m_dest, 0, 0, m_width, 0xff000000);
    Drawing::DrawVLine(m_dest, 0, 0, m_height, 0xff000000);
    Drawing::DrawHLine(m_dest, m_height, 0, m_width, 0xff000000);
    Drawing::DrawVLine(m_dest, m_width, 0, m_height, 0xff000000);
  }
} /* namespace MFM */

