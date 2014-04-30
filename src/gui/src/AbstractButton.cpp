#include "AbstractButton.h"
#include "SDL/SDL.h"
#include "Drawing.h"

namespace MFM
{
  AbstractButton::AbstractButton() : m_text(0), m_enabled(true)
  { }

  AbstractButton::AbstractButton(const char* text) : m_text(text), m_enabled(true)
  { }

  AbstractButton::~AbstractButton()
  {}

  void AbstractButton::Render(Drawing & drawing, SPoint& offset, TTF_Font* font)
  {
    drawing.SetForeground(BUTTON_BORDER_COLOR);

    SPoint loc = m_location + offset;
    UPoint floc = MakeUnsigned(loc);
    floc.SetX(floc.GetX() + 20);
    drawing.FillRect(loc.GetX() - BUTTON_BORDER_SIZE,
                     loc.GetY() - BUTTON_BORDER_SIZE,
                     m_dimensions.GetX() + 2 * BUTTON_BORDER_SIZE,
                     m_dimensions.GetY() + 2 * BUTTON_BORDER_SIZE);
    drawing.FillRect(loc.GetX(),
                     loc.GetY(),
                     m_dimensions.GetX(),
                     m_dimensions.GetY(),
                     BUTTON_COLOR);

    drawing.SetFont(font);
    drawing.BlitText(m_text, floc, MakeUnsigned(m_dimensions));
  }

  void AbstractButton::PaintComponent(Drawing & drawing)
  {
    // Fade fg and bg toward each other if not enabled.
    drawing.SetForeground(m_enabled? m_fgColor : Drawing::InterpolateColors(m_fgColor,m_bgColor,60));
    drawing.SetBackground(m_enabled? m_bgColor : Drawing::InterpolateColors(m_fgColor,m_bgColor,40));
    drawing.Clear();

    drawing.BlitText(m_text, UPoint(0,0), GetDimensions());
  }

  bool AbstractButton::Contains(SPoint& pt)
  {
    if(pt.GetX() > m_location.GetX() &&
       pt.GetY() > m_location.GetY() &&
       pt.GetX() < m_location.GetX() + m_dimensions.GetX() &&
       pt.GetY() < m_location.GetY() + m_dimensions.GetY())
    {
      return true;
    }
    return false;
  }
}
