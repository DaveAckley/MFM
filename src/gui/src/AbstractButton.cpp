#include "AbstractButton.h"
#include "SDL.h"
#include "Drawing.h"

namespace MFM
{
  void AbstractButton::Init()
  {
    this->SetForeground(BUTTON_COLOR);
    this->SetBorder(BUTTON_BORDER_COLOR);
    this->SetBackground(BUTTON_BACKGROUND_COLOR);
  }

  AbstractButton::AbstractButton() :
    m_text(0),
    m_icon(0),
    m_enabled(true),
    m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(const char* text) :
    m_text(text),
    m_icon(0),
    m_enabled(true),
    m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(SDL_Surface* icon) :
    m_text(0),
    m_icon(icon),
    m_enabled(true),
    m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(const char* text, SDL_Surface* icon) :
    m_text(text),
    m_icon(icon),
    m_enabled(true),
    m_justClicked(false)
  {
    Init();
  }

  AbstractButton::~AbstractButton()
  { }

  void AbstractButton::PaintComponent(Drawing& d)
  {
    if(!PaintClickHighlight(d))
    {
      PaintComponentNonClick(d);
    }
  }

  void AbstractButton::PaintComponentNonClick(Drawing & drawing)
  {
    // Fade fg and bg toward each other if not enabled.
    drawing.SetForeground(m_enabled? m_fgColor : Drawing::InterpolateColors(m_fgColor,m_bgColor,60));
    drawing.SetBackground(m_enabled? m_bgColor : Drawing::InterpolateColors(m_fgColor,m_bgColor,40));
    drawing.Clear();

    if(m_icon)
    {
      drawing.BlitImage(m_icon, UPoint(0,0), Panel::GetDimensions());
    }

    if(m_text)
    {
      SPoint dims = MakeSigned(Panel::GetDimensions());
      SPoint textSize = Panel::GetTextSize(drawing.GetFont(), m_text);
      SPoint renderAt = max((dims-textSize)/2, SPoint(0,0));

      drawing.BlitText(m_text, MakeUnsigned(renderAt), GetDimensions());
    }
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
